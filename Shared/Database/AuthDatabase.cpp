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
#include <Database/AuthDatabase.hpp>

namespace Database
{
    AuthDatabase *AuthDatabase::m_instance = nullptr;

    AuthDatabase::AuthDatabase() : Connection("127.0.0.1", 3306, "root", "root", "auth") {}

    AuthDatabase *AuthDatabase::instance()
    {
        if (!m_instance)
            m_instance = new AuthDatabase();
        return m_instance;
    }

} // namespace Database
