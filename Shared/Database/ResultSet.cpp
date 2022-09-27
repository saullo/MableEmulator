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
#include <Database/ResultSet.hpp>
#include <Utilities/Log.hpp>

namespace Database
{
    ResultSet::ResultSet(MYSQL_RES *result, MYSQL_FIELD *fields, std::uint64_t row_count, std::uint32_t field_count)
        : m_result(result), m_fields(fields), m_row_count(row_count), m_field_count(field_count)
    {
        m_field_data.resize(m_field_count);
        m_current_row = new Field[m_field_count];

        for (std::uint32_t i = 0; i < m_field_count; i++)
        {
            auto meta = &m_field_data[i];
            auto field = &m_fields[i];

            meta->table_name = field->org_table;
            meta->table_alias = field->table;
            meta->name = field->org_name;
            meta->alias = field->name;
            meta->index = i;
            meta->type = mysql_type_to_field_type(field->type);
            meta->type_name = field_type_string(field->type);

            m_current_row[i].set_metadata(&m_field_data[i]);
        }
    }

    ResultSet ::~ResultSet() { clear(); }

    const char *ResultSet::field_type_string(enum_field_types type)
    {
        switch (type)
        {
        case MYSQL_TYPE_BIT:
            return "BIT";
        case MYSQL_TYPE_BLOB:
            return "BLOB";
        case MYSQL_TYPE_DATE:
            return "DATE";
        case MYSQL_TYPE_DATETIME:
            return "DATETIME";
        case MYSQL_TYPE_NEWDECIMAL:
            return "NEWDECIMAL";
        case MYSQL_TYPE_DECIMAL:
            return "DECIMAL";
        case MYSQL_TYPE_DOUBLE:
            return "DOUBLE";
        case MYSQL_TYPE_ENUM:
            return "ENUM";
        case MYSQL_TYPE_FLOAT:
            return "FLOAT";
        case MYSQL_TYPE_GEOMETRY:
            return "GEOMETRY";
        case MYSQL_TYPE_INT24:
            return "INT24";
        case MYSQL_TYPE_LONG:
            return "LONG";
        case MYSQL_TYPE_LONGLONG:
            return "LONGLONG";
        case MYSQL_TYPE_LONG_BLOB:
            return "LONG_BLOB";
        case MYSQL_TYPE_MEDIUM_BLOB:
            return "MEDIUM_BLOB";
        case MYSQL_TYPE_NEWDATE:
            return "NEWDATE";
        case MYSQL_TYPE_NULL:
            return "NULL";
        case MYSQL_TYPE_SET:
            return "SET";
        case MYSQL_TYPE_SHORT:
            return "SHORT";
        case MYSQL_TYPE_STRING:
            return "STRING";
        case MYSQL_TYPE_TIME:
            return "TIME";
        case MYSQL_TYPE_TIMESTAMP:
            return "TIMESTAMP";
        case MYSQL_TYPE_TINY:
            return "TINY";
        case MYSQL_TYPE_TINY_BLOB:
            return "TINY_BLOB";
        case MYSQL_TYPE_VAR_STRING:
            return "VAR_STRING";
        case MYSQL_TYPE_YEAR:
            return "YEAR";
        default:
            return "UNKNOWN";
        }
    }

    DatabaseFieldTypes ResultSet::mysql_type_to_field_type(enum_field_types type)
    {
        switch (type)
        {
        case MYSQL_TYPE_NULL:
            return DatabaseFieldTypes::Null;
        case MYSQL_TYPE_TINY:
            return DatabaseFieldTypes::Int8;
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_SHORT:
            return DatabaseFieldTypes::Int16;
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
            return DatabaseFieldTypes::Int32;
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_BIT:
            return DatabaseFieldTypes::Int64;
        case MYSQL_TYPE_FLOAT:
            return DatabaseFieldTypes::Float;
        case MYSQL_TYPE_DOUBLE:
            return DatabaseFieldTypes::Double;
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            return DatabaseFieldTypes::Decimal;
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
            return DatabaseFieldTypes::Date;
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
            return DatabaseFieldTypes::Binary;
        default:
            LOG_WARN("Invalid mysql field type = {}", std::uint32_t(type));
            break;
        }

        return DatabaseFieldTypes::Null;
    }

    Field *ResultSet::fetch() { return m_current_row; }

    bool ResultSet::next_row()
    {
        if (!m_result)
            return false;

        auto row = mysql_fetch_row(m_result);
        if (!row)
        {
            clear();
            return false;
        }

        auto lengths = mysql_fetch_lengths(m_result);
        if (!lengths)
        {
            LOG_ERROR("Failed to retrive lengths value, error = {}", mysql_error(m_result->handle));
            clear();
            return false;
        }

        for (std::uint32_t i = 0; i < m_field_count; i++)
            m_current_row[i].set_data(row[i], lengths[i]);

        return true;
    }

    const Field &ResultSet::operator[](std::size_t index) const
    {
        assert(index < m_field_count);
        return m_current_row[index];
    }

    void ResultSet::clear()
    {
        if (m_current_row)
        {
            delete[] m_current_row;
            m_current_row = nullptr;
        }

        if (m_result)
        {
            mysql_free_result(m_result);
            m_result = nullptr;
        }
    }
} // namespace Database
