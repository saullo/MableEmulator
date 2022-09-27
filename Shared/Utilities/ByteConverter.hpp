#pragma once

#include <algorithm>

namespace Utilities
{
    namespace ByteConverter
    {
        template <std::size_t T> inline void convert(char *value)
        {
            std::swap(*value, *(value + T - 1));
            convert<T - 2>(value + 1);
        }

        template <> inline void convert<0>(char *) {}
        template <> inline void convert<1>(char *) {}

        template <typename T> inline void apply(T *value) { convert<sizeof(T)>((char *)(value)); }
    } // namespace ByteConverter

    template <typename T> inline void endian_convert(T &) {}
    template <typename T> inline void endian_convert_reverse(T &value) { ByteConverter::apply<T>(&value); }
} // namespace Utilities
