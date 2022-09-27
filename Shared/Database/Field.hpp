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
#include <array>
#include <cstdint>
#include <string>

namespace Database
{
    class ResultSet;
    class Field
    {
        friend class ResultSet;

    public:
        float get_float();
        std::uint8_t get_uint8();
        std::uint16_t get_uint16();
        std::uint32_t get_uint32();
        std::string get_string() const;
        const char *get_c_string() const;

        template <std::size_t Size> std::array<std::uint8_t, Size> get_binary() const
        {
            std::array<std::uint8_t, Size> buffer;
            get_binary_sized(buffer.data(), Size);
            return buffer;
        }

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

        void get_binary_sized(std::uint8_t *buffer, std::size_t length) const;
    };
} // namespace Database
