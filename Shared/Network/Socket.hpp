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
#pragma once

#include <Utilities/ByteBuffer.hpp>
#include <Utilities/Log.hpp>
#include <Utilities/MessageBuffer.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>
#include <queue>

namespace Network
{
    template <class T> class Socket : public std::enable_shared_from_this<T>
    {
    public:
        Socket(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket)), m_timer(m_socket.get_executor())
        {
            m_timer.expires_at(std::chrono::steady_clock::time_point::max());
        }

        void start()
        {
            on_start();

            boost::asio::co_spawn(
                m_socket.get_executor(), [self = this->shared_from_this()] { return self->reader(); },
                boost::asio::detached);

            boost::asio::co_spawn(
                m_socket.get_executor(), [self = this->shared_from_this()] { return self->writer(); },
                boost::asio::detached);
        }

    protected:
        auto remote_address() { return m_socket.remote_endpoint().address(); }
        auto remote_port() { return m_socket.remote_endpoint().port(); }

        auto &read_buffer() { return m_read_buffer; }

        void close_socket()
        {
            m_socket.close();
            m_timer.cancel();
        }

        void queue_packet(Utilities::MessageBuffer &&packet)
        {
            m_write_queue.push(std::move(packet));
            m_timer.cancel_one();
        }

        virtual void on_start() {}
        virtual void on_read() {}

    private:
        bool m_writing_async{false};
        boost::asio::ip::tcp::socket m_socket;
        boost::asio::steady_timer m_timer;
        Utilities::MessageBuffer m_read_buffer;
        std::queue<Utilities::MessageBuffer> m_write_queue;

        boost::asio::awaitable<void> reader()
        {
            try
            {
                while (true)
                {
                    m_read_buffer.normalize();
                    m_read_buffer.ensure_free_space();

                    auto length = co_await m_socket.async_read_some(
                        boost::asio::buffer(m_read_buffer.write_ptr(), m_read_buffer.remaining_size()),
                        boost::asio::use_awaitable);

                    m_read_buffer.write_completed(length);
                    on_read();
                }
            }
            catch (const std::exception &e)
            {
                LOG_CRITICAL("Unhandled reader exception - {}", e.what());
                close_socket();
            }
        }

        boost::asio::awaitable<void> writer()
        {
            try
            {
                while (m_socket.is_open())
                {
                    if (m_writing_async || m_write_queue.empty())
                    {
                        boost::system::error_code error;
                        co_await m_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, error));
                    }

                    while (handle_queue())
                        ;
                }
            }
            catch (const std::exception &e)
            {
                LOG_CRITICAL("Unhandled writer exception - {}", e.what());
                close_socket();
            }
        }

        bool handle_queue()
        {
            if (m_write_queue.empty())
                return false;

            auto &message = m_write_queue.front();
            auto message_size = message.active_size();

            boost::system::error_code error;
            auto length = m_socket.write_some(boost::asio::buffer(message.read_ptr(), message_size), error);

            if (error)
            {
                if (error == boost::asio::error::would_block || error == boost::asio::error::try_again)
                    return handle_queue_async();
                m_write_queue.pop();
                return false;
            }
            else if (length == 0)
            {
                m_write_queue.pop();
                return false;
            }
            else if (length < message_size)
            {
                message.read_completed(length);
                return handle_queue_async();
            }

            m_write_queue.pop();
            return !m_write_queue.empty();
        }

        bool handle_queue_async()
        {
            if (m_writing_async)
                return false;
            m_writing_async = true;

            m_socket.async_write_some(boost::asio::null_buffers(), [this](auto, auto) {
                m_writing_async = false;
                handle_queue();
            });
            return false;
        }
    };
} // namespace Network
