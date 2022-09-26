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
#include <Database/Field.hpp>
#include <cstdlib>

namespace Database
{
    std::uint32_t Field::get_uint32()
    {
        if (!m_data.value)
            return 0;

        if (m_data.is_raw)
            return *reinterpret_cast<const std::uint32_t *>(m_data.value);
        return static_cast<std::uint32_t>(std::strtoul(m_data.value, nullptr, 10));
    }

    void Field::set_data(const char *value, std::uint32_t length)
    {
        m_data.value = value;
        m_data.length = length;
        m_data.is_raw = false;
    }

    void Field::set_metadata(const QueryResultField *metadata) { m_metadata = metadata; }
} // namespace Database
