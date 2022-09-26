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
#include <Utilities/Log.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>

namespace Authentication
{
    std::array<std::uint8_t, 16> Session::version_challenge = {
        {0xBA, 0xA3, 0x1E, 0x99, 0xA0, 0x0B, 0x21, 0x57, 0xFC, 0x37, 0x3F, 0xB3, 0x69, 0xCD, 0xD2, 0xF1}};

    Session::Session(boost::asio::ip::tcp::socket socket)
        : m_socket(std::move(socket)), m_timer(m_socket.get_executor())
    {
        m_timer.expires_at(std::chrono::steady_clock::time_point::max());
    }

    void Session::start()
    {
        auto endpoint = m_socket.remote_endpoint();
        LOG_DEBUG("Connected: {}:{}", endpoint.address().to_string(), endpoint.port());

        boost::asio::co_spawn(
            m_socket.get_executor(), [self = this->shared_from_this()] { return self->reader(); },
            boost::asio::detached);

        boost::asio::co_spawn(
            m_socket.get_executor(), [self = this->shared_from_this()] { return self->writer(); },
            boost::asio::detached);
    }

    boost::asio::awaitable<void> Session::reader()
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
                read_handler();
            }
        }
        catch (const std::exception &e)
        {
            LOG_CRITICAL("Unhandled reader exception - {}", e.what());
            stop();
        }
    }

    boost::asio::awaitable<void> Session::writer()
    {
        try
        {
            while (m_socket.is_open())
            {
                if (m_write_queue.empty())
                {
                    boost::system::error_code error;
                    co_await m_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, error));
                }
                else
                {
                    auto &message = m_write_queue.front();
                    auto message_size = message.active_size();
                    auto length = co_await m_socket.async_write_some(
                        boost::asio::buffer(message.read_ptr(), message_size), boost::asio::use_awaitable);

                    if (message_size < length)
                        message.read_completed(length);

                    m_write_queue.pop();
                }
            }
        }
        catch (const std::exception &e)
        {
            LOG_CRITICAL("Unhandled writer exception - {}", e.what());
            stop();
        }
    }

    void Session::stop()
    {
        m_socket.close();
        m_timer.cancel();
    }

    void Session::read_handler()
    {
        Handler command_handlers[] = {
            {cmd_auth_logon_challenge, logon_challenge_initial_size, &Session::logon_challenge_handler}};
        auto handlers_size = sizeof(command_handlers) / sizeof(Handler);

        auto &read_buffer = m_read_buffer;
        while (read_buffer.active_size())
        {
            Handler handler;
            std::size_t i;
            auto command = read_buffer.read_ptr()[0];
            for (i = 0; i < handlers_size; i++)
            {
                handler = command_handlers[i];
                if (handler.command == command)
                    break;
            }

            if (i == handlers_size)
            {
                read_buffer.reset();
                LOG_DEBUG("Received invalid command = {}", command);
                return;
            }

            auto size = std::uint16_t(handler.size);
            if (command == cmd_auth_logon_challenge)
            {
                auto challenge = reinterpret_cast<cmd_auth_logon_challenge_client_t *>(read_buffer.read_ptr());
                size += challenge->size;
                if (size > sizeof(cmd_auth_logon_challenge_client_t) + 16)
                {
                    stop();
                    return;
                }
            }

            if (read_buffer.active_size() < size)
                break;

            if (!(*this.*handler.handler)())
            {
                LOG_DEBUG("Command handler failed, command = {}", command);
                stop();
                return;
            }

            read_buffer.read_completed(size);
        }
    }

    bool Session::logon_challenge_handler()
    {
        auto challenge = reinterpret_cast<cmd_auth_logon_challenge_client_t *>(m_read_buffer.read_ptr());
        if (challenge->size - (sizeof(cmd_auth_logon_challenge_client_t) - logon_challenge_initial_size - 1) !=
            challenge->account_name_length)
            return false;

        Utilities::ByteBuffer buffer;
        buffer << cmd_auth_logon_challenge;
        buffer << std::uint8_t(0x00);

        if (!build_is_valid(challenge->build))
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

        account_query->next_row();
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

        auto endpoint = m_socket.remote_endpoint();
        LOG_DEBUG("Successfully logged account username = {}, address = {}:{}", username,
                  endpoint.address().to_string(), endpoint.port());

        send_packet(buffer);
        return true;
    }

    bool Session::build_is_valid(std::uint16_t build_number)
    {
        std::vector<BuildInformation> builds;

        if (auto query = Database::AuthDatabase::instance()->query(
                "SELECT build, major, minor, revision FROM build_information"))
        {
            do
            {
                auto fields = query->fetch();
                auto &build = builds.emplace_back();
                build.build = fields[0].get_uint32();
                build.major = fields[1].get_uint32();
                build.minor = fields[2].get_uint32();
                build.revision = fields[3].get_uint32();
            } while (query->next_row());
        }

        for (const auto &build : builds)
        {
            if (build.build == build_number)
                return true;
        }
        return false;
    }

    void Session::send_packet(Utilities::ByteBuffer &packet)
    {
        if (packet.empty())
            return;

        Utilities::MessageBuffer buffer(packet.size());
        buffer.write(packet.data(), packet.size());
        queue_packet(std::move(buffer));
    }

    void Session::queue_packet(Utilities::MessageBuffer &&packet)
    {
        m_write_queue.push(std::move(packet));
        m_timer.cancel_one();
    }
} // namespace Authentication
