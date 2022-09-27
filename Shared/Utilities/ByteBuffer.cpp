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
#include <Utilities/ByteBuffer.hpp>
#include <cassert>
#include <cstring>

namespace Utilities
{
    ByteBuffer::ByteBuffer() { m_data.reserve(initial_size); }

    std::size_t ByteBuffer::size() { return m_data.size(); }

    std::uint8_t *ByteBuffer::data() { return m_data.data(); }
    const std::uint8_t *ByteBuffer::data() const { return m_data.data(); }

    bool ByteBuffer::empty() { return m_data.empty(); }

    ByteBuffer &ByteBuffer::operator<<(float value)
    {
        append<float>(value);
        return *this;
    }

    ByteBuffer &ByteBuffer::operator<<(std::uint8_t value)
    {
        append<std::uint8_t>(value);
        return *this;
    }

    ByteBuffer &ByteBuffer::operator<<(std::uint16_t value)
    {
        append<std::uint16_t>(value);
        return *this;
    }

    ByteBuffer &ByteBuffer::operator<<(std::uint32_t value)
    {
        append<std::uint32_t>(value);
        return *this;
    }

    ByteBuffer &ByteBuffer::operator<<(const std::string &value) { return operator<<(std::string_view(value)); }

    ByteBuffer &ByteBuffer::operator<<(std::string_view value)
    {
        if (auto length = value.length())
            append(reinterpret_cast<const std::uint8_t *>(value.data()), length);
        append(static_cast<std::uint8_t>(0));
        return *this;
    }

    template <typename T> void ByteBuffer::append(T value)
    {
        static_assert(std::is_fundamental<T>::value);
        append((std::uint8_t *)&value, sizeof(value));
    }

    void ByteBuffer::append(const std::uint8_t *value, std::size_t size)
    {
        assert(value);
        assert(size);
        assert(this->size() < 10000000);

        auto new_size = m_write_pos + size;
        if (m_data.capacity() < new_size)
        {
            if (new_size < 100)
                m_data.reserve(300);
            else if (new_size < 750)
                m_data.reserve(2500);
            else if (new_size < 6000)
                m_data.reserve(10000);
            else
                m_data.reserve(400000);
        }

        if (m_data.size() < new_size)
            m_data.resize(new_size);

        std::memcpy(&m_data[m_write_pos], value, size);
        m_write_pos = new_size;
    }

    void ByteBuffer::append(const ByteBuffer &buffer)
    {
        if (!buffer.m_write_pos)
            return;

        append(buffer.data(), buffer.m_write_pos);
    }

    void ByteBuffer::resize(std::size_t new_size)
    {
        m_data.resize(new_size, 0);
        m_read_pos = 0;
        m_write_pos = size();
    }
} // namespace Utilities
