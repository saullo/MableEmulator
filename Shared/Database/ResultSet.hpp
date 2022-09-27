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

#include <Database/Field.hpp>
#include <Database/QueryResultField.hpp>
#include <mysql/mysql.h>
#include <vector>

namespace Database
{
    class ResultSet
    {
    public:
        ResultSet(ResultSet const &right) = delete;
        ResultSet &operator=(ResultSet const &right) = delete;
        ResultSet(MYSQL_RES *result, MYSQL_FIELD *fields, std::uint64_t row_count, std::uint32_t field_count);
        ~ResultSet();

        auto row_count() { return m_row_count; }

        Field *fetch();
        bool next_row();

        const Field &operator[](std::size_t index) const;

    private:
        MYSQL_RES *m_result;
        MYSQL_FIELD *m_fields;
        Field *m_current_row;
        std::uint32_t m_field_count;
        std::uint64_t m_row_count;
        std::vector<QueryResultField> m_field_data;

        void clear();
        DatabaseFieldTypes mysql_type_to_field_type(enum_field_types type);
        const char *field_type_string(enum_field_types type);
    };
} // namespace Database
