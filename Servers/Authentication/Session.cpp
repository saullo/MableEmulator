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
#include <Database/AuthDatabase.hpp>
#include <Realm/RealmList.hpp>
#include <boost/lexical_cast.hpp>

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
            {cmd_auth_logon_proof, sizeof(cmd_auth_logon_proof_client_t), &Session::logon_proof_handler},
            {cmd_realmlist, realmlist_packet_size, &Session::realmlist_handler}};
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

            LOG_DEBUG("Command = {}", command);

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

        m_build = challenge->build;
        m_expansion = calculate_expansion_version(m_build);

        Utilities::ByteBuffer buffer;
        buffer << std::uint8_t(cmd_auth_logon_challenge);
        buffer << std::uint8_t(0x00);

        if (!Realm::RealmList::instance()->build_info(m_build))
        {
            buffer << std::uint8_t(login_version_invalid);
            send_packet(buffer);
            return false;
        }

        auto username =
            std::string(reinterpret_cast<const char *>(challenge->account_name), challenge->account_name_length);
        auto account_query_sql =
            fmt::format("SELECT id, username, salt, verifier FROM account WHERE username = '{}';", username);
        auto account_query = Database::AuthDatabase::instance()->query(account_query_sql.c_str());
        if (!account_query)
        {
            buffer << std::uint8_t(login_unknown_account);
            send_packet(buffer);
            return false;
        }

        auto fields = account_query->fetch();
        m_account.load(fields);

        m_srp6.emplace(m_account.username, fields[2].get_binary<Crypto::Srp6::salt_length>(),
                       fields[3].get_binary<Crypto::Srp6::verifier_length>());

        buffer << std::uint8_t(login_ok);
        buffer.append(m_srp6->B);
        buffer << std::uint8_t(1);
        buffer.append(m_srp6->g);
        buffer << std::uint8_t(32);
        buffer.append(m_srp6->N);
        buffer.append(m_srp6->s);
        buffer.append(version_challenge.data(), version_challenge.size());
        buffer << std::uint8_t(0x00);

        LOG_DEBUG("Account username = {}, address = {}:{}", m_account.username, remote_address().to_string(),
                  remote_port());

        send_packet(buffer);
        return true;
    }

    bool Session::logon_proof_handler()
    {
        if (m_expansion == expansion_flag_invalid)
        {
            LOG_DEBUG("Invalid client version, expansion = {}", m_expansion);
            return false;
        }

        auto logon_proof = reinterpret_cast<cmd_auth_logon_proof_client_t *>(read_buffer().read_ptr());
        if (auto key = m_srp6->verify_challenge(logon_proof->client_public_key, logon_proof->client_proof))
        {
            m_session_key = *key;

            auto sent_token = (logon_proof->security_flags & 0x04);
            if (sent_token)
            {
                Utilities::ByteBuffer buffer;
                buffer << std::uint8_t(cmd_auth_logon_proof);
                buffer << std::uint8_t(login_unknown_account);
                buffer << std::uint16_t(0x0);
                send_packet(buffer);
                return true;
            }

            auto server_proof = Crypto::Srp6::session_verifier(logon_proof->client_public_key,
                                                               logon_proof->client_proof, m_session_key);

            LOG_DEBUG("Successfully logged account username = {}, address = {}:{}", m_account.username,
                      remote_address().to_string(), remote_port());

            Utilities::ByteBuffer buffer;
            if (m_expansion & expansion_flag_post_bc)
            {
                cmd_auth_logon_proof_server_pos_t proof;
                proof.command = cmd_auth_logon_proof;
                proof.result = 0;
                proof.server_proof = server_proof;
                proof.account_flag = 0x00800000;
                proof.hardware_survey_id = 0x00;
                proof.unknown_flags = 0x00;

                buffer.resize(sizeof(proof));
                std::memcpy(buffer.data(), &proof, sizeof(proof));
            }
            else
            {
                cmd_auth_logon_proof_server_pre_t proof;
                proof.command = cmd_auth_logon_proof;
                proof.result = 0;
                proof.server_proof = server_proof;
                proof.hardware_survey_id = 0x00;

                buffer.resize(sizeof(proof));
                std::memcpy(buffer.data(), &proof, sizeof(proof));
            }

            send_packet(buffer);
        }
        else
        {
            Utilities::ByteBuffer buffer;
            buffer << std::uint8_t(cmd_auth_logon_proof);
            buffer << std::uint8_t(login_unknown_account);
            buffer << std::uint16_t(0);
            send_packet(buffer);
        }
        return true;
    }

    bool Session::realmlist_handler()
    {
        std::map<std::uint32_t, std::uint8_t> characters;
        auto character_query =
            fmt::format("SELECT realm_id, count FROM characters WHERE account_id = '{}'", m_account.id);
        if (auto query = Database::AuthDatabase::instance()->query(character_query.c_str()))
        {
            do
            {
                auto fields = query->fetch();
                characters[fields[0].get_uint32()] = fields[1].get_uint8();
            } while (query->next_row());
        }

        Utilities::ByteBuffer realmlist_buffer;
        std::size_t realmlist_size = 0;
        auto realm_list = Realm::RealmList::instance();
        for (const auto &realm_map : realm_list->realms())
        {
            const auto &realm = realm_map.second;

            std::uint32_t flags = realm.flags;
            const auto build_info = realm_list->build_info(realm.build);
            if (m_build != realm.build)
            {
                if (!build_info)
                    continue;
                flags |= Realm::realmflag_offline | Realm::realmflag_specifybuild;
            }
            if (!build_info)
                flags &= ~Realm::realmflag_specifybuild;

            auto name = realm.name;
            if (m_expansion & expansion_flag_pre_bc && flags & Realm::realmflag_specifybuild)
                name = fmt::format("{} ({}.{}.{})", name, build_info->major, build_info->minor, build_info->revision);

            if (m_expansion & expansion_flag_post_bc)
            {
                realmlist_buffer << std::uint8_t(realm.type);
                realmlist_buffer << std::uint8_t(0x01);
            }
            else
                realmlist_buffer << std::uint32_t(realm.type);
            realmlist_buffer << std::uint8_t(flags);
            realmlist_buffer << name;
            realmlist_buffer << boost::lexical_cast<std::string>(realm.address_for_client(remote_address()));
            realmlist_buffer << float(realm.population);
            realmlist_buffer << std::uint8_t(characters[realm.id]);
            realmlist_buffer << std::uint8_t(realm.category);
            if (m_expansion & expansion_flag_post_bc)
                realmlist_buffer << std::uint8_t(realm.id);
            else
                realmlist_buffer << std::uint8_t(0x00);

            if (m_expansion & expansion_flag_post_bc && flags & Realm::realmflag_specifybuild)
            {
                realmlist_buffer << std::uint8_t(build_info->major);
                realmlist_buffer << std::uint8_t(build_info->minor);
                realmlist_buffer << std::uint8_t(build_info->revision);
                realmlist_buffer << std::uint16_t(build_info->build);
            }

            realmlist_size++;
        }

        if (m_expansion & expansion_flag_post_bc)
        {
            realmlist_buffer << std::uint8_t(0x10);
            realmlist_buffer << std::uint8_t(0x00);
        }
        else
        {
            realmlist_buffer << std::uint8_t(0x00);
            realmlist_buffer << std::uint8_t(0x02);
        }

        Utilities::ByteBuffer realmlist_size_buffer;
        realmlist_size_buffer << std::uint32_t(0x00);
        if (m_expansion & expansion_flag_post_bc)
            realmlist_size_buffer << std::uint16_t(realmlist_size);
        else
            realmlist_size_buffer << std::uint8_t(realmlist_size);

        Utilities::ByteBuffer header_buffer;
        header_buffer << std::uint8_t(cmd_realmlist);
        header_buffer << std::uint16_t(realmlist_buffer.size() + realmlist_size_buffer.size());
        header_buffer.append(realmlist_size_buffer);
        header_buffer.append(realmlist_buffer);
        send_packet(header_buffer);

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

    void Session::Account::load(Database::Field *field)
    {
        id = field[0].get_uint32();
        username = field[1].get_string();
    }

    std::uint8_t Session::calculate_expansion_version(std::uint32_t build)
    {
        auto realm_list = Realm::RealmList::instance();
        if (realm_list->is_pre_bc_client(build))
            return expansion_flag_pre_bc;
        if (realm_list->is_post_bc_client(build))
            return expansion_flag_post_bc;
        return expansion_flag_invalid;
    }
} // namespace Authentication
