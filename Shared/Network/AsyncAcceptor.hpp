/*
 * World of Warcraft server emulator
 * Copyright (C) 2022 Saullo Bretas Silva
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <utility>

namespace Network
{
    class AsyncAcceptor
    {
    public:
        typedef void (*AcceptCallback)(boost::asio::ip::tcp::socket &&socket, std::uint32_t);

        AsyncAcceptor(boost::asio::io_context &io_context, const std::string &ip, int port)
            : m_acceptor(io_context), m_endpoint(boost::asio::ip::make_address(ip), port)
        {
        }

        bool bind()
        {
            boost::system::error_code code;
            m_acceptor.open(m_endpoint.protocol(), code);
            if (code)
            {
                std::cerr << "Failed to open acceptor - " << code.message().c_str() << std::endl;
                return false;
            }

            m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), code);
            if (code)
            {
                std::cerr << "Failed to set acceptor::reuse_address - " << code.message().c_str() << std::endl;
                return false;
            }

            m_acceptor.bind(m_endpoint, code);
            if (code)
            {
                std::cerr << "Failed to bind acceptor - " << code.message().c_str() << std::endl;
                return false;
            }

            m_acceptor.listen(boost::asio::socket_base::max_listen_connections, code);
            if (code)
            {
                std::cerr << "Failed start listening - " << code.message().c_str() << std::endl;
                return false;
            }

            return true;
        }

        template <AcceptCallback accept_callback> void async_accept_with_callback()
        {
            boost::asio::ip::tcp::socket *socket;
            std::uint32_t index;
            std::tie(socket, index) = m_socket_factory();
            m_acceptor.async_accept(*socket, [this, socket, index](boost::system::error_code error) {
                if (!error)
                {
                    try
                    {
                        socket->non_blocking(true);
                        accept_callback(std::move(*socket), index);
                    }
                    catch (const boost::system::system_error &e)
                    {
                        std::cerr << "Failed to initialize socket - " << e.what() << std::endl;
                    }
                }

                if (!m_closed)
                    this->template async_accept_with_callback<accept_callback>();
            });
        }

        void set_socket_factory(std::function<std::pair<boost::asio::ip::tcp::socket *, std::uint32_t>()> func)
        {
            m_socket_factory = std::move(func);
        }

    private:
        std::atomic<bool> m_closed{false};
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::endpoint m_endpoint;
        std::function<std::pair<boost::asio::ip::tcp::socket *, std::uint32_t>()> m_socket_factory;
    };
} // namespace Network
