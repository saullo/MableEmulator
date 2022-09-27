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
#include <Authentication/SessionManager.hpp>

namespace Authentication
{
    SessionManager *SessionManager::m_instance = nullptr;

    SessionManager *SessionManager::instance()
    {
        if (!m_instance)
            m_instance = new SessionManager();
        return m_instance;
    }

    bool SessionManager::init(boost::asio::io_context &io_context, const std::string &ip, int port, int thread_count)
    {
        if (!Network::SocketManager<Session>::init(io_context, ip, port, thread_count))
            return false;
        m_acceptor->async_accept_with_callback<&SessionManager::on_socket_accept>();
        return true;
    }

    void SessionManager::on_socket_accept(boost::asio::ip::tcp::socket &&socket, std::uint32_t index)
    {
        instance()->on_socket_open(std::forward<boost::asio::ip::tcp::socket>(socket), index);
    }

    Network::Thread<Session> *SessionManager::create_threads() const { return new Network::Thread<Session>[1]; }
} // namespace Authentication
