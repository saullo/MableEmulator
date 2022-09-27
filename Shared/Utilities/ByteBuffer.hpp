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

#include <array>
#include <cstdint>
#include <vector>

namespace Utilities
{
    class ByteBuffer
    {
    public:
        ByteBuffer();

        std::size_t size();
        std::uint8_t *data();
        bool empty();

        ByteBuffer &operator<<(std::uint8_t value);

        template <typename T> void append(T value);
        void append(const std::uint8_t *value, std::size_t size);
        template <std::size_t Size> void append(const std::array<std::uint8_t, Size> &value)
        {
            append(value.data(), Size);
        }

        void resize(std::size_t new_size);

    private:
        static constexpr auto initial_size = 4096;

        std::vector<std::uint8_t> m_data;
        std::size_t m_write_pos{0};
        std::size_t m_read_pos{0};
    };
} // namespace Utilities
