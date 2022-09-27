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
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <queue>

namespace Network
{
    template <class T> class Socket : public std::enable_shared_from_this<T>
    {
    public:
        Socket(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket)) {}

        auto is_open() { return !m_closed && !m_closing; }

        void start() { on_start(); }

        bool update()
        {
            if (m_closed)
                return false;

            if (m_writing_async || (m_write_queue.empty() && !m_closing))
                return true;

            while (handle_queue())
                ;

            on_update();

            return true;
        }

        void close_socket()
        {
            if (m_closed.exchange(true))
                return;

            boost::system::error_code code;
            m_socket.shutdown(boost::asio::socket_base::shutdown_send, code);
            if (code)
                LOG_ERROR("Error on remote = {}, socket shutdown, code = {}, message = {}",
                          remote_address().to_string(), code.value(), code.message());
        }

    protected:
        virtual void on_start() {}
        virtual void on_update() {}
        virtual void on_read() {}

        auto remote_address() { return m_socket.remote_endpoint().address(); }
        auto remote_port() { return m_socket.remote_endpoint().port(); }

        auto &read_buffer() { return m_read_buffer; }

        void queue_packet(Utilities::MessageBuffer &&packet) { m_write_queue.push(std::move(packet)); }

        void async_read()
        {
            if (!is_open())
                return;

            m_read_buffer.normalize();
            m_read_buffer.ensure_free_space();
            m_socket.async_read_some(boost::asio::buffer(m_read_buffer.write_ptr(), m_read_buffer.remaining_size()),
                                     [this](boost::system::error_code error, std::size_t bytes) {
                                         if (error)
                                         {
                                             close_socket();
                                             return;
                                         }

                                         m_read_buffer.write_completed(bytes);
                                         on_read();
                                     });
        }

    private:
        std::atomic<bool> m_closed{false};
        std::atomic<bool> m_closing{false};
        bool m_writing_async{false};
        boost::asio::ip::tcp::socket m_socket;
        Utilities::MessageBuffer m_read_buffer;
        std::queue<Utilities::MessageBuffer> m_write_queue;

        bool handle_queue()
        {
            if (m_write_queue.empty())
                return false;

            auto &message = m_write_queue.front();
            auto message_size = message.active_size();

            boost::system::error_code error;
            auto sent = m_socket.write_some(boost::asio::buffer(message.read_ptr(), message_size), error);

            if (error)
            {
                if (error == boost::asio::error::would_block || error == boost::asio::error::try_again)
                    return handle_queue_async();
                m_write_queue.pop();
                if (m_closing && m_write_queue.empty())
                    close_socket();
                return false;
            }
            else if (sent == 0)
            {
                m_write_queue.pop();
                if (m_closing && m_write_queue.empty())
                    close_socket();
                return false;
            }
            else if (sent < message_size)
            {
                message.read_completed(sent);
                return handle_queue_async();
            }

            m_write_queue.pop();
            if (m_closing && m_write_queue.empty())
                close_socket();

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
