#include <Database/Field.hpp>
#include <cstdlib>

namespace Database
{
    std::uint32_t Field::get_uint32()
    {
        if (!m_data.value)
            return 0;

        if (m_data.is_raw)
            return *reinterpret_cast<const std::uint32_t *>(m_data.value);
        return static_cast<std::uint32_t>(std::strtoul(m_data.value, nullptr, 10));
    }

    void Field::set_data(const char *value, std::uint32_t length)
    {
        m_data.value = value;
        m_data.length = length;
        m_data.is_raw = false;
    }

    void Field::set_metadata(const QueryResultField *metadata) { m_metadata = metadata; }
} // namespace Database
