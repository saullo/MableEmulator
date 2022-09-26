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
