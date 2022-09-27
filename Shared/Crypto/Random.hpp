#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <openssl/rand.h>

namespace Crypto
{
    void get_random_bytes(std::uint8_t *buffer, std::size_t length)
    {
        auto result = RAND_bytes(buffer, length);
        assert(result == 1);
    }

    template <typename Container> void get_random_bytes(Container &container)
    {
        get_random_bytes(std::data(container), std::size(container));
    }

    template <std::size_t Size> std::array<std::uint8_t, Size> get_random_bytes()
    {
        std::array<std::uint8_t, Size> result;
        get_random_bytes(result);
        return result;
    }
} // namespace Crypto
