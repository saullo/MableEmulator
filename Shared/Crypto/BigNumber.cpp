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
#include <Crypto/BigNumber.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>

namespace Crypto
{
    BigNumber::BigNumber() : m_bn(BN_new()) {}

    BigNumber::BigNumber(const BigNumber &bn) : m_bn(BN_dup(bn.bn())) {}

    BigNumber::BigNumber(std::int32_t value) : BigNumber() { set_dword(value); }

    BigNumber::BigNumber(std::uint32_t value) : BigNumber() { set_dword(value); }

    BigNumber::~BigNumber() { BN_free(m_bn); }

    void BigNumber::set_binary(const std::uint8_t *bytes, std::int32_t length, bool little_endian)
    {
        if (little_endian)
        {
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x10100000L
            auto array = new std::uint8_t[length];
            for (int i = 0; i < length; i++)
                array[i] = bytes[length - 1 - i];

            BN_bin2bn(array, length, m_bn);
            delete[] array;
#else
            BN_lebin2bn(bytes, length, m_bn);
#endif
        }
        else
            BN_bin2bn(bytes, length, m_bn);
    }

    void BigNumber::set_dword(std::uint32_t value) { BN_set_word(m_bn, value); }

    void BigNumber::set_dword(std::int32_t value)
    {
        set_dword(std::uint32_t(std::abs(value)));
        if (value < 0)
            BN_set_negative(m_bn, 1);
    }

    void BigNumber::get_bytes(std::uint8_t *buffer, std::size_t length, bool little_endian) const
    {
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x10100000L
        auto number_bytes = number_of_bytes();
        assert(number_bytes >= 0);

        std::size_t bytes_count = static_cast<std::size_t>(number_bytes);
        assert(bytes_count <= length);

        if (bytes_count < length)
            std::memset(reinterpret_cast<void *>(buffer), 0, length);

        BN_bn2bin(m_bn, buffer + (length - bytes_count));

        if (little_endian)
            std::reverse(buffer, buffer + length);
#else
        auto result = little_endian ? BN_bn2lebinpad(m_bn, buffer, length) : BN_bn2binpad(m_bn, buffer, length);
        assert(result > 0);
#endif
    }

    std::int32_t BigNumber::number_of_bytes() const { return BN_num_bytes(m_bn); }

    BigNumber BigNumber::mod_exp(const BigNumber &bn1, const BigNumber &bn2) const
    {
        BigNumber result;
        BN_CTX *context = BN_CTX_new();
        BN_mod_exp(result.m_bn, m_bn, bn1.m_bn, bn2.m_bn, context);
        BN_CTX_free(context);
        return result;
    }

    BigNumber &BigNumber::operator=(const BigNumber &bn)
    {
        if (this == &bn)
            return *this;

        BN_copy(m_bn, bn.m_bn);
        return *this;
    }

    BigNumber &BigNumber::operator+=(const BigNumber &bn)
    {
        BN_add(m_bn, m_bn, bn.m_bn);
        return *this;
    }

    BigNumber BigNumber::operator+(const BigNumber &bn) const
    {
        BigNumber result(*this);
        return result += bn;
    }

    BigNumber &BigNumber::operator-=(const BigNumber &bn)
    {
        BN_sub(m_bn, m_bn, bn.m_bn);
        return *this;
    }

    BigNumber BigNumber::operator-(const BigNumber &bn) const
    {
        BigNumber result(*this);
        return result -= bn;
    }

    BigNumber &BigNumber::operator*=(const BigNumber &bn)
    {
        BN_CTX *context = BN_CTX_new();
        BN_mul(m_bn, m_bn, bn.m_bn, context);
        BN_CTX_free(context);
        return *this;
    }

    BigNumber BigNumber::operator*(const BigNumber &bn) const
    {
        BigNumber result(*this);
        return result *= bn;
    }

    BigNumber &BigNumber::operator/=(const BigNumber &bn)
    {
        BN_CTX *bnctx = BN_CTX_new();
        BN_div(m_bn, nullptr, m_bn, bn.m_bn, bnctx);
        BN_CTX_free(bnctx);
        return *this;
    }

    BigNumber BigNumber::operator/(const BigNumber &bn) const
    {
        BigNumber result(*this);
        return result /= bn;
    }

    BigNumber &BigNumber::operator%=(const BigNumber &bn)
    {
        BN_CTX *context = BN_CTX_new();
        BN_mod(m_bn, m_bn, bn.m_bn, context);
        BN_CTX_free(context);
        return *this;
    }

    BigNumber BigNumber::operator%(const BigNumber &bn) const
    {
        BigNumber result(*this);
        return result %= bn;
    }

    BigNumber &BigNumber::operator<<=(int number)
    {
        BN_lshift(m_bn, m_bn, number);
        return *this;
    }

    BigNumber BigNumber::operator<<(int number) const
    {
        BigNumber result(*this);
        return result <<= number;
    }

} // namespace Crypto
