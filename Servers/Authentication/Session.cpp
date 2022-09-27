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
#include <Authentication/RealmList.hpp>
#include <Authentication/Session.hpp>
#include <Database/AuthDatabase.hpp>

namespace Authentication
{
    std::array<std::uint8_t, 16> Session::version_challenge = {
        {0xBA, 0xA3, 0x1E, 0x99, 0xA0, 0x0B, 0x21, 0x57, 0xFC, 0x37, 0x3F, 0xB3, 0x69, 0xCD, 0xD2, 0xF1}};

    Session::Session(boost::asio::ip::tcp::socket socket) : Socket(std::move(socket)) {}

    void Session::on_start() { LOG_DEBUG("Connected: {}:{}", remote_address().to_string(), remote_port()); }

    void Session::on_read()
    {
        Handler command_handlers[] = {
            {cmd_auth_logon_challenge, logon_challenge_initial_size, &Session::logon_challenge_handler},
            {cmd_auth_logon_proof, sizeof(cmd_auth_logon_proof_client_t), &Session::logon_proof_handler}};
        auto handlers_size = sizeof(command_handlers) / sizeof(Handler);

        auto &buffer = read_buffer();
        while (buffer.active_size())
        {
            Handler handler;
            std::size_t i;
            auto command = buffer.read_ptr()[0];
            for (i = 0; i < handlers_size; i++)
            {
                handler = command_handlers[i];
                if (handler.command == command)
                    break;
            }

            if (i == handlers_size)
            {
                buffer.reset();
                LOG_DEBUG("Received invalid command = {}", command);
                return;
            }

            auto size = std::uint16_t(handler.size);
            if (command == cmd_auth_logon_challenge)
            {
                auto challenge = reinterpret_cast<cmd_auth_logon_challenge_client_t *>(buffer.read_ptr());
                size += challenge->size;
                if (size > sizeof(cmd_auth_logon_challenge_client_t) + 16)
                {
                    close_socket();
                    return;
                }
            }

            if (buffer.active_size() < size)
                break;

            if (!(*this.*handler.handler)())
            {
                LOG_DEBUG("Command handler failed, command = {}", command);
                close_socket();
                return;
            }

            buffer.read_completed(size);
        }
    }

    bool Session::logon_challenge_handler()
    {
        auto challenge = reinterpret_cast<cmd_auth_logon_challenge_client_t *>(read_buffer().read_ptr());
        if (challenge->size - (sizeof(cmd_auth_logon_challenge_client_t) - logon_challenge_initial_size - 1) !=
            challenge->account_name_length)
            return false;

        Utilities::ByteBuffer buffer;
        buffer << cmd_auth_logon_challenge;
        buffer << std::uint8_t(0x00);

        if (!RealmList::instance()->build_info(challenge->build))
        {
            buffer << login_version_invalid;
            send_packet(buffer);
            return false;
        }

        auto username =
            std::string(reinterpret_cast<const char *>(challenge->account_name), challenge->account_name_length);
        auto account_query_sql =
            fmt::format("SELECT username, salt, verifier FROM account WHERE username = '{}';", username);
        auto account_query = Database::AuthDatabase::instance()->query(account_query_sql.c_str());
        if (!account_query)
        {
            buffer << login_unknown_account;
            send_packet(buffer);
            return false;
        }

        auto fields = account_query->fetch();

        m_srp6.emplace(fields[0].get_string(), fields[1].get_binary<Crypto::Srp6::salt_length>(),
                       fields[2].get_binary<Crypto::Srp6::verifier_length>());

        buffer << login_ok;
        buffer.append(m_srp6->B);
        buffer << std::uint8_t(1);
        buffer.append(m_srp6->g);
        buffer << std::uint8_t(32);
        buffer.append(m_srp6->N);
        buffer.append(m_srp6->s);
        buffer.append(version_challenge.data(), version_challenge.size());
        buffer << std::uint8_t(0x00);

        LOG_DEBUG("Successfully logged account username = {}, address = {}:{}", username, remote_address().to_string(),
                  remote_port());

        send_packet(buffer);
        return true;
    }

    bool Session::logon_proof_handler()
    {
        auto logon_proof = reinterpret_cast<cmd_auth_logon_proof_client_t *>(read_buffer().read_ptr());
        if (auto key = m_srp6->verify_challenge(logon_proof->client_public_key, logon_proof->client_proof))
        {
            m_session_key = *key;

            auto server_proof = Crypto::Srp6::session_verifier(logon_proof->client_public_key,
                                                               logon_proof->client_proof, m_session_key);

            cmd_auth_logon_proof_server_t proof = {};
            proof.command = cmd_auth_logon_proof;
            proof.result = 0;
            proof.server_proof = server_proof;
            proof.hardware_survey_id = 0x00;

            Utilities::ByteBuffer buffer;
            buffer.resize(sizeof(proof));
            std::memcpy(buffer.data(), &proof, sizeof(proof));

            send_packet(buffer);
        }
        else
        {
            Utilities::ByteBuffer buffer;
            buffer << cmd_auth_logon_proof;
            buffer << login_unknown_account;
            buffer << std::uint16_t(0);
            send_packet(buffer);
        }
        return true;
    }

    void Session::send_packet(Utilities::ByteBuffer &packet)
    {
        if (packet.empty())
            return;

        Utilities::MessageBuffer buffer(packet.size());
        buffer.write(packet.data(), packet.size());
        queue_packet(std::move(buffer));
    }
} // namespace Authentication
