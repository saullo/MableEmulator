/**
 * MableEmulator is a server emulator for World of Warcraft
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
#include <Authentication/Session.hpp>
#include <Utilities/Log.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>

namespace Authentication
{
    Session::Session(boost::asio::ip::tcp::socket socket)
        : m_socket(std::move(socket)), m_timer(m_socket.get_executor())
    {
        m_timer.expires_at(std::chrono::steady_clock::time_point::max());
    }

    void Session::start()
    {
        auto endpoint = m_socket.remote_endpoint();
        LOG_DEBUG("Connected: {}:{}", endpoint.address().to_string(), endpoint.port());

        boost::asio::co_spawn(
            m_socket.get_executor(), [self = this->shared_from_this()] { return self->reader(); },
            boost::asio::detached);

        boost::asio::co_spawn(
            m_socket.get_executor(), [self = this->shared_from_this()] { return self->writer(); },
            boost::asio::detached);
    }

    boost::asio::awaitable<void> Session::reader()
    {
        try
        {
            std::vector<std::uint8_t> buffer;
            buffer.resize(1024);

            while (true)
            {
                auto length =
                    co_await m_socket.async_read_some(boost::asio::buffer(buffer), boost::asio::use_awaitable);
                LOG_DEBUG("Message length = {}", length);

                m_queue.push_back(buffer);
                m_timer.cancel_one();
            }
        }
        catch (const std::exception &e)
        {
            LOG_CRITICAL("Unhandled reader exception - {}", e.what());
            stop();
        }
    }

    boost::asio::awaitable<void> Session::writer()
    {
        try
        {
            while (m_socket.is_open())
            {
                if (m_queue.empty())
                {
                    boost::system::error_code error;
                    co_await m_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, error));
                }
                else
                {
                    auto message = m_queue.front();
                    co_await m_socket.async_write_some(boost::asio::buffer(message), boost::asio::use_awaitable);
                    m_queue.pop_front();
                }
            }
        }
        catch (const std::exception &e)
        {
            LOG_CRITICAL("Unhandled writer exception - {}", e.what());
            stop();
        }
    }

    void Session::stop()
    {
        m_socket.close();
        m_timer.cancel();
    }
} // namespace Authentication
