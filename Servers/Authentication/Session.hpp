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
#include <Utilities/MessageBuffer.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>
#include <queue>

namespace Authentication
{
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        Session(boost::asio::ip::tcp::socket socket);

        void start();

    private:
        enum Command : std::uint8_t
        {
            cmd_auth_logon_challenge = 0x00
        };

        enum Result : std::uint8_t
        {
            login_ok = 0x00,
            login_unknown_account = 0x04,
        };

        struct Handler
        {
            Command command;
            std::size_t size;
            bool (Session::*handler)();
        };

#pragma pack(push, 1)
        typedef struct
        {
            std::uint8_t opcode;
            std::uint8_t protocol_version;
            std::uint16_t size;
            std::uint8_t game_name[4];
            std::uint8_t version[3];
            std::uint16_t build;
            std::uint8_t platform[4];
            std::uint8_t os[4];
            std::uint8_t locale[4];
            std::uint32_t worldregion_bias;
            std::uint32_t ip;
            std::uint8_t account_name_length;
            std::uint8_t account_name[1];
        } cmd_auth_logon_challenge_client_t;
        static_assert(sizeof(cmd_auth_logon_challenge_client_t) ==
                      (1 + 1 + 2 + 4 + 1 + 1 + 1 + 2 + 4 + 4 + 4 + 4 + 4 + 1 + 1));
#pragma pack(pop)

        Utilities::MessageBuffer m_read_buffer;
        boost::asio::ip::tcp::socket m_socket;
        boost::asio::steady_timer m_timer;
        std::queue<Utilities::MessageBuffer> m_write_queue;

        void stop();
        boost::asio::awaitable<void> reader();
        boost::asio::awaitable<void> writer();

        void read_handler();
        bool logon_challenge_handler();
        void send_packet(Utilities::ByteBuffer &packet);
        void queue_packet(Utilities::MessageBuffer &&packet);
    };
} // namespace Authentication
