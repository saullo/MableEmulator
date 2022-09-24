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

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <deque>
#include <memory>

namespace Authentication
{
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        Session(boost::asio::ip::tcp::socket socket);

        void start();

    private:
        boost::asio::ip::tcp::socket m_socket;
        boost::asio::steady_timer m_timer;
        std::deque<std::vector<std::uint8_t>> m_queue;

        void stop();
        boost::asio::awaitable<void> reader();
        boost::asio::awaitable<void> writer();
    };
} // namespace Authentication
