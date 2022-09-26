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

#include <Database/ResultSet.hpp>
#include <cstdint>
#include <mysql/mysql.h>

namespace Database
{
    class Connection
    {
    public:
        Connection(const char *host, int port, const char *user, const char *password, const char *database);

        std::uint32_t open();
        void close();
        ResultSet *query(const char *sql);

    private:
        MYSQL *m_handler;
        int m_port{-1};
        const char *m_host{nullptr};
        const char *m_user{nullptr};
        const char *m_password{nullptr};
        const char *m_database{nullptr};
    };
} // namespace Database
