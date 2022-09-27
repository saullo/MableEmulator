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

#include <Crypto/Srp6.hpp>
#include <Database/Field.hpp>
#include <Network/Socket.hpp>

namespace Authentication
{
    class Session : public Network::Socket<Session>
    {
    public:
        Session(boost::asio::ip::tcp::socket socket);

    protected:
        void on_start() override;
        void on_read() override;

    private:
        static constexpr auto realmlist_packet_size = 5;
        static constexpr auto logon_challenge_initial_size = 4;
        static std::array<std::uint8_t, 16> version_challenge;

        enum Command
        {
            cmd_auth_logon_challenge = 0x00,
            cmd_auth_logon_proof = 0x01,
            cmd_realmlist = 0x10
        };

        enum Result
        {
            login_ok = 0x00,
            login_unknown_account = 0x04,
            login_version_invalid = 0x09,
        };

        enum ExpansionFlags
        {
            expansion_flag_invalid = 0x00,
            expansion_flag_pre_bc = 0x01,
            expansion_flag_post_bc = 0x02,
        };

        struct Handler
        {
            Command command;
            std::size_t size;
            bool (Session::*handler)();
        };

        struct Account
        {
            std::uint32_t id{0};
            std::string username;

            void load(Database::Field *field);
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

        typedef struct
        {
            std::uint8_t command;
            Crypto::Srp6::EphemeralKey client_public_key;
            Crypto::SHA1::Digest client_proof;
            Crypto::SHA1::Digest crc_hash;
            std::uint8_t num_keys;
            std::uint8_t security_flags;
        } cmd_auth_logon_proof_client_t;
        static_assert(sizeof(cmd_auth_logon_proof_client_t) == (1 + 32 + 20 + 20 + 1 + 1));

        typedef struct
        {
            std::uint8_t command;
            std::uint8_t result;
            Crypto::SHA1::Digest server_proof;
            std::uint32_t hardware_survey_id;
        } cmd_auth_logon_proof_server_pre_t;
        static_assert(sizeof(cmd_auth_logon_proof_server_pre_t) == (1 + 1 + 20 + 4));

        typedef struct
        {
            std::uint8_t command;
            std::uint8_t result;
            Crypto::SHA1::Digest server_proof;
            std::uint32_t account_flag;
            std::uint32_t hardware_survey_id;
            std::uint16_t unknown_flags;
        } cmd_auth_logon_proof_server_pos_t;
        static_assert(sizeof(cmd_auth_logon_proof_server_pos_t) == (1 + 1 + 20 + 4 + 4 + 2));
#pragma pack(pop)
        std::uint16_t m_build{0};
        std::optional<Crypto::Srp6> m_srp6;
        Crypto::Srp6::SessionKey m_session_key{};
        Account m_account{};
        std::uint8_t m_expansion{expansion_flag_invalid};

        bool logon_challenge_handler();
        bool logon_proof_handler();
        bool realmlist_handler();
        void send_packet(Utilities::ByteBuffer &packet);
        std::uint8_t calculate_expansion_version(std::uint32_t build);
    };
} // namespace Authentication
