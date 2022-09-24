#include <Utilities/ByteBuffer.hpp>
#include <cassert>
#include <cstring>

namespace Utilities
{
    ByteBuffer::ByteBuffer() { m_data.reserve(initial_size); }

    std::size_t ByteBuffer::size() { return m_data.size(); }

    std::uint8_t *ByteBuffer::data() { return m_data.data(); }

    bool ByteBuffer::empty() { return m_data.empty(); }

    ByteBuffer &ByteBuffer::operator<<(std::uint8_t value)
    {
        append<std::uint8_t>(value);
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

} // namespace Utilities