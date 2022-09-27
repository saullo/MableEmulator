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

#include <Authentication/Session.hpp>
#include <Network/SocketManager.hpp>

namespace Authentication
{
    class SessionManager : public Network::SocketManager<Session>
    {
    public:
        static SessionManager *instance();

        bool init(boost::asio::io_context &io_context, const std::string &ip, int port, int thread_count) override;

    protected:
        [[nodiscard]] Network::Thread<Session> *create_threads() const override;

    private:
        static SessionManager *m_instance;

        static void on_socket_accept(boost::asio::ip::tcp::socket &&socket, std::uint32_t index);
    };
} // namespace Authentication
