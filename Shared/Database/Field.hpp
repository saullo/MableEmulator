#pragma once

#include <Database/QueryResultField.hpp>
#include <cstdint>

namespace Database
{
    class ResultSet;
    class Field
    {
        friend class ResultSet;

    public:
        std::uint32_t get_uint32();

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
    };
} // namespace Database
