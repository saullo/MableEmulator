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
#include <vector>

namespace Utilities
{
    class MessageBuffer
    {
    public:
        MessageBuffer();
        MessageBuffer(std::size_t size);

        std::uint8_t *base_ptr();
        std::uint8_t *write_ptr();
        std::uint8_t *read_ptr();
        std::size_t active_size();
        std::size_t remaining_size();
        void reset();
        void normalize();
        void ensure_free_space();
        void read_completed(std::size_t size);
        void write_completed(std::size_t size);
        void write(const void *data, std::size_t size);
        void resize(std::size_t size);
        std::vector<std::uint8_t> &&move();

    private:
        static constexpr auto initial_size = 4096;

        std::vector<std::uint8_t> m_data;
        std::size_t m_write_pos{0};
        std::size_t m_read_pos{0};
    };
} // namespace Utilities
