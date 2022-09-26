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

#include <Database/QueryResultField.hpp>
#include <cstdint>

namespace Database
{
    class ResultSet;
    class Field
    {
        friend class ResultSet;

    public:
        std::uint32_t get_uint32();

    protected:
        void set_data(const char *value, std::uint32_t length);
        void set_metadata(const QueryResultField *metadata);

    private:
        struct
        {
            const char *value{nullptr};
            std::uint32_t length{0};
            bool is_raw{false};
        } m_data{};

        const QueryResultField *m_metadata;
    };
} // namespace Database
