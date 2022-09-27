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
#include <Crypto/Random.hpp>
#include <Crypto/Srp6.hpp>
#include <algorithm>
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

    SHA1::Digest Srp6::session_verifier(const EphemeralKey &p_a, const SHA1::Digest &p_m, const SessionKey &key)
    {
        return SHA1::digest_of(p_a, p_m, key);
    }

    std::optional<Srp6::SessionKey> Srp6::verify_challenge(const EphemeralKey &p_a, const SHA1::Digest &p_m)
    {
        assert(!m_used);
        m_used = true;

        const BigNumber a(p_a);
        if ((a % m_N).is_zero())
            return std::nullopt;

        const BigNumber u(SHA1::digest_of(p_a, B));
        const EphemeralKey S = (a * (m_v.mod_exp(u, m_N))).mod_exp(m_b, N).to_byte_array<32>();
        auto key = SHA1_inter_leave(S);
        const SHA1::Digest n_hash = SHA1::digest_of(N);
        const SHA1::Digest g_hash = SHA1::digest_of(g);

        SHA1::Digest ng_hash;
        std::transform(n_hash.begin(), n_hash.end(), g_hash.begin(), ng_hash.begin(), std::bit_xor<>());

        const SHA1::Digest m = SHA1::digest_of(ng_hash, m_I, s, p_a, B, key);
        if (m == p_m)
            return key;
        return std::nullopt;
    }

    Srp6::SessionKey Srp6::SHA1_inter_leave(const EphemeralKey &S)
    {
        std::array<std::uint8_t, ephemeral_key_length / 2> buffer0;
        std::array<std::uint8_t, ephemeral_key_length / 2> buffer1;
        for (std::size_t i = 0; i < ephemeral_key_length / 2; i++)
        {
            buffer0[i] = S[2 * i + 0];
            buffer1[i] = S[2 * i + 1];
        }

        std::size_t p = 0;
        while (p < ephemeral_key_length && !S[p])
            p++;

        if (p & 1)
            p++;
        p /= 2;

        const SHA1::Digest hash0 = SHA1::digest_of(buffer0.data() + p, ephemeral_key_length / 2 - p);
        const SHA1::Digest hash1 = SHA1::digest_of(buffer1.data() + p, ephemeral_key_length / 2 - p);

        SessionKey key;
        for (size_t i = 0; i < SHA1::disgest_length; i++)
        {
            key[2 * i + 0] = hash0[i];
            key[2 * i + 1] = hash1[i];
        }
        return key;
    }
} // namespace Crypto
