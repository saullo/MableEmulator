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

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <thread>

namespace Network
{
    template <class SocketType> class Thread
    {
    public:
        Thread() : m_io_context(1), m_accept_socket(m_io_context), m_update_timer(m_io_context) {}

        ~Thread()
        {
            stop();
            if (m_thread)
            {
                wait();
                delete m_thread;
            }
        }

        int connection_count() { return m_connections; }

        bool start()
        {
            if (m_thread)
                return false;

            m_thread = new std::thread(&Thread::run, this);
            return true;
        }

        void append(std::shared_ptr<SocketType> socket)
        {
            std::lock_guard<std::mutex> lock(m_new_sockets_lock);
            m_connections++;
            m_new_sockets.push_back(socket);
        }

        auto get_socket_for_accept() { return &m_accept_socket; }

    private:
        typedef boost::posix_time::ptime Time;
        typedef boost::asio::time_traits<boost::posix_time::ptime> TimeTraits;
        typedef boost::asio::io_context::executor_type Executor;

        std::atomic<bool> m_stopped{false};
        std::atomic<int> m_connections{0};
        std::thread *m_thread{nullptr};
        std::mutex m_new_sockets_lock;
        std::vector<std::shared_ptr<SocketType>> m_sockets;
        std::vector<std::shared_ptr<SocketType>> m_new_sockets;
        boost::asio::io_context m_io_context;
        boost::asio::ip::tcp::socket m_accept_socket;
        boost::asio::basic_deadline_timer<Time, TimeTraits, Executor> m_update_timer;

        void run()
        {
            m_update_timer.expires_from_now(boost::posix_time::milliseconds(1));
            m_update_timer.async_wait([this](const boost::system::error_code &code) { update(); });

            m_io_context.run();

            m_new_sockets.clear();
        }

        void update()
        {
            if (m_stopped)
                return;

            m_update_timer.expires_from_now(boost::posix_time::milliseconds(1));
            m_update_timer.async_wait([this](const boost::system::error_code &code) { update(); });

            add_new_sockets();

            auto remove = [this](std::shared_ptr<SocketType> socket) -> bool {
                if (!socket->update())
                {
                    if (socket->is_open())
                        socket->close_socket();

                    this->m_connections--;
                    return true;
                }
                return false;
            };

            m_sockets.erase(std::remove_if(m_sockets.begin(), m_sockets.end(), remove), m_sockets.end());
        }

        void add_new_sockets()
        {
            std::lock_guard<std::mutex> lock(m_new_sockets_lock);
            if (m_new_sockets.empty())
                return;

            for (auto socket : m_new_sockets)
            {
                if (!socket->is_open())
                    m_connections--;
                else
                    m_sockets.push_back(socket);
            }
            m_new_sockets.clear();
        }

        void stop()
        {
            m_stopped = true;
            m_io_context.stop();
        }

        void wait()
        {
            assert(m_thread);
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }
    };
} // namespace Network
