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
#include <Database/Connection.hpp>
#include <Utilities/Log.hpp>

namespace Database
{
    Connection::Connection(const char *host, int port, const char *user, const char *password, const char *database)
        : m_host(host), m_port(port), m_user(user), m_password(password), m_database(database)
    {
    }

    std::uint32_t Connection::open()
    {
        auto init = mysql_init(nullptr);
        if (!init)
        {
            LOG_ERROR("Failed to initialize MySQL");
            return CR_UNKNOWN_ERROR;
        }

        mysql_options(init, MYSQL_SET_CHARSET_NAME, "utf8");

        m_handler = mysql_real_connect(init, m_host, m_user, m_password, m_database, m_port, nullptr, 0);
        if (!m_handler)
        {
            LOG_ERROR("Connection failed with host = {}, port = {}, user = {}, database = {}, error = {}", m_host,
                      m_port, m_user, m_database, mysql_error(init));
            auto error_code = mysql_errno(init);
            mysql_close(init);
            return error_code;
        }

        LOG_DEBUG("Successfully connected to MySQL Database host = {}, port = {}, database = {}", m_host, m_port,
                  m_database);
        mysql_autocommit(m_handler, true);
        mysql_set_character_set(m_handler, "utf8");
        return 0;
    }

    void Connection::close()
    {
        if (m_handler)
        {
            mysql_close(m_handler);
            m_handler = nullptr;
        }
    }

    ResultSet *Connection::query(const char *sql)
    {
        if (!sql)
            return nullptr;

        if (!m_handler)
            return nullptr;

        if (mysql_query(m_handler, sql) != 0)
        {
            LOG_ERROR("Failed to query sql = {}, error = {}", sql, mysql_error(m_handler));
            return nullptr;
        }
        else
            LOG_DEBUG("Successfully query sql = {}", sql);

        auto result = mysql_store_result(m_handler);
        if (!result)
            return nullptr;

        auto row_count = mysql_affected_rows(m_handler);
        if (!row_count)
            return nullptr;

        auto field_count = mysql_field_count(m_handler);
        auto fields = mysql_fetch_fields(result);
        auto result_set = new ResultSet(result, fields, row_count, field_count);
        if (!result_set || !result_set->row_count() || !result_set->next_row())
        {
            delete result_set;
            return nullptr;
        }
        return result_set;
    }

    bool Connection::execute(const char *sql)
    {
        if (!m_handler)
            return false;

        if (mysql_query(m_handler, sql) != 0)
        {
            LOG_ERROR("Failed to execute sql = {}, error = {}", sql, mysql_error(m_handler));
            return false;
        }
        else
            LOG_DEBUG("Successfully execute sql = {}", sql);
        return true;
    }
} // namespace Database
