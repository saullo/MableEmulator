#pragma once

#include <cstdint>

namespace Database
{
    enum class DatabaseFieldTypes : std::uint8_t
    {
        Null,
        Int8,
        Int16,
        Int32,
        Int64,
        Float,
        Double,
        Decimal,
        Date,
        Binary
    };

    struct QueryResultField
    {
        const char *table_name{nullptr};
        const char *table_alias{nullptr};
        const char *name{nullptr};
        const char *alias{nullptr};
        const char *type_name{nullptr};
        std::uint32_t index{0};
        DatabaseFieldTypes type{DatabaseFieldTypes::Null};
    };
} // namespace Database
