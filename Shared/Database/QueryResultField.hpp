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
