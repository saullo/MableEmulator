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

    private:
        static constexpr auto initial_size = 4096;

        std::vector<std::uint8_t> m_data;
        std::size_t m_write_pos{0};
        std::size_t m_read_pos{0};
    };
} // namespace Utilities