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

#include <Network/AsyncAcceptor.hpp>
#include <Network/Thread.hpp>

namespace Network
{
    template <class SocketType> class SocketManager
    {
    public:
        virtual bool init(boost::asio::io_context &io_context, const std::string &ip, int port, int thread_count)
        {
            AsyncAcceptor *acceptor;
            try
            {
                acceptor = new AsyncAcceptor(io_context, ip, port);
            }
            catch (const boost::system::system_error &e)
            {
                return false;
            }

            if (!acceptor->bind())
            {
                delete acceptor;
                return false;
            }

            m_acceptor = acceptor;
            m_thread_count = thread_count;
            m_threads = create_threads();

            for (auto i = 0; i < m_thread_count; i++)
                m_threads[i].start();

            acceptor->set_socket_factory([this]() { return get_socket_for_accept(); });

            return true;
        }

        void on_socket_open(boost::asio::ip::tcp::socket &&socket, std::uint32_t index)
        {
            try
            {
                auto new_socket = std::make_shared<SocketType>(std::move(socket));
                new_socket->start();
                m_threads[index].append(new_socket);
            }
            catch (const boost::system::system_error &e)
            {
                std::cerr << "Failed to retrieve client socket - " << e.what() << std::endl;
            }
        }

    protected:
        auto thread_count() const { return m_thread_count; }

        AsyncAcceptor *m_acceptor{nullptr};

        virtual Thread<SocketType> *create_threads() const = 0;

    private:
        Thread<SocketType> *m_threads{nullptr};
        int m_thread_count{0};

        std::pair<boost::asio::ip::tcp::socket *, std::uint32_t> get_socket_for_accept()
        {
            auto index = thread_with_min_connections();
            return std::make_pair(m_threads[index].get_socket_for_accept(), index);
        }

        auto thread_with_min_connections() const
        {
            std::uint32_t min = 0;
            for (int i = 1; i < m_thread_count; i++)
            {
                if (m_threads[i].connection_count() < m_threads[min].connection_count())
                    min = i;
            }
            return min;
        }
    };
} // namespace Network
