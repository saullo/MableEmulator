#pragma once

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

    private:
        static constexpr auto initial_size = 4096;

        std::vector<std::uint8_t> m_data;
        std::size_t m_write_pos{0};
        std::size_t m_read_pos{0};

        template <typename T> void append(T value);
        void append(const std::uint8_t *value, std::size_t size);
    };
} // namespace Utilities