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
#include <Utilities/MessageBuffer.hpp>
#include <cstring>

namespace Utilities
{
    MessageBuffer::MessageBuffer() { m_data.resize(initial_size); }

    MessageBuffer::MessageBuffer(std::size_t size) { m_data.resize(size); }

    std::uint8_t *MessageBuffer::base_ptr() { return m_data.data(); }

    std::uint8_t *MessageBuffer::write_ptr() { return base_ptr() + m_write_pos; }

    std::uint8_t *MessageBuffer::read_ptr() { return base_ptr() + m_read_pos; }

    std::size_t MessageBuffer::active_size() { return m_write_pos - m_read_pos; }

    std::size_t MessageBuffer::remaining_size() { return m_data.size() - m_write_pos; }

    void MessageBuffer::reset()
    {
        m_write_pos = 0;
        m_read_pos = 0;
    }

    void MessageBuffer::normalize()
    {
        if (!m_read_pos)
            return;

        if (m_read_pos != m_write_pos)
            std::memmove(base_ptr(), read_ptr(), active_size());

        m_write_pos -= m_read_pos;
        m_read_pos = 0;
    }

    void MessageBuffer::ensure_free_space()
    {
        if (remaining_size() > 0)
            return;

        m_data.resize(m_data.size() + initial_size);
    }

    void MessageBuffer::read_completed(std::size_t size) { m_read_pos += size; }

    void MessageBuffer::write_completed(std::size_t size) { m_write_pos += size; }

    void MessageBuffer::write(const void *data, std::size_t size)
    {
        if (!size)
            return;

        std::memcpy(write_ptr(), data, size);
        write_completed(size);
    }

    void MessageBuffer::resize(std::size_t size) { m_data.resize(size); }

    std::vector<std::uint8_t> &&MessageBuffer::move()
    {
        m_write_pos = 0;
        m_read_pos = 0;
        return std::move(m_data);
    }
} // namespace Utilities
