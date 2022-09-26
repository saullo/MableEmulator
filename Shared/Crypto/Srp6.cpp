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
#include <Crypto/Srp6.hpp>
#include <cassert>
#include <openssl/rand.h>

namespace Crypto
{
    namespace Details
    {
        void hex_string_to_byte_array(std::string_view value, std::uint8_t *result, std::size_t length, bool reverse)
        {
            assert(value.size() == (2 * length));

            std::int32_t start = 0;
            std::int8_t operation = 1;
            auto end = std::int32_t(value.length());
            if (reverse)
            {
                start = std::int32_t(value.length() - 2);
                end = -2;
                operation = -1;
            }

            std::uint32_t j = 0;
            for (std::int32_t i = start; i != end; i += 2 * operation)
            {
                char buffer[3] = {value[i], value[i + 1], '\0'};
                result[j++] = std::uint8_t(std::strtoul(buffer, nullptr, 16));
            }
        }
    } // namespace Details

    template <size_t Size>
    void hex_string_to_byte_array(std::string_view value, std::array<std::uint8_t, Size> &result, bool reverse = false)
    {
        Details::hex_string_to_byte_array(value, result.data(), Size, reverse);
    }

    template <std::size_t Size>
    std::array<std::uint8_t, Size> hex_string_to_byte_array(std::string_view value, bool reverse = false)
    {
        std::array<std::uint8_t, Size> result;
        hex_string_to_byte_array(value, result, reverse);
        return result;
    }

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

    const std::array<std::uint8_t, 1> Srp6::g = {7};
    const std::array<std::uint8_t, 32> Srp6::N =
        hex_string_to_byte_array<32>("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7", true);
    const BigNumber Srp6::m_g(Srp6::g);
    const BigNumber Srp6::m_N(Srp6::N);

    Srp6::Srp6(const std::string &username, const Salt &salt, const Verifier &verifier)
        : m_I(SHA1::digest_of(username)), m_b(get_random_bytes<32>()), m_v(verifier), s(salt), B(calculate_B(m_b, m_v))
    {
    }

    Srp6::EphemeralKey Srp6::calculate_B(const BigNumber &b, const BigNumber &v)
    {
        return ((m_g.mod_exp(b, m_N) + (v * 3)) % N).to_byte_array<ephemeral_key_length>();
    }
} // namespace Crypto
