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
#pragma once

#include <array>
#include <cstdint>
#include <openssl/bn.h>

namespace Crypto
{
    class BigNumber
    {
    public:
        BigNumber();
        BigNumber(const BigNumber &bn);
        BigNumber(std::int32_t value);
        BigNumber(std::uint32_t value);
        template <std::size_t Size>
        BigNumber(const std::array<std::uint8_t, Size> &value, bool little_endian = true) : BigNumber()
        {
            set_binary(value.data(), Size, little_endian);
        }
        ~BigNumber();

        auto bn() { return m_bn; }
        const auto bn() const { return m_bn; }

        BigNumber mod_exp(const BigNumber &bn1, const BigNumber &bn2) const;

        template <std::size_t Size> std::array<std::uint8_t, Size> to_byte_array(bool little_endian = true) const
        {
            std::array<std::uint8_t, Size> result;
            get_bytes(result.data(), Size, little_endian);
            return result;
        }

        bool is_zero() const;

        BigNumber &operator=(BigNumber const &bn);
        BigNumber &operator+=(const BigNumber &bn);
        BigNumber operator+(const BigNumber &bn) const;
        BigNumber &operator-=(const BigNumber &bn);
        BigNumber operator-(const BigNumber &bn) const;
        BigNumber &operator*=(const BigNumber &bn);
        BigNumber operator*(const BigNumber &bn) const;
        BigNumber &operator/=(const BigNumber &bn);
        BigNumber operator/(const BigNumber &bn) const;
        BigNumber &operator%=(const BigNumber &bn);
        BigNumber operator%(const BigNumber &bn) const;
        BigNumber &operator<<=(int number);
        BigNumber operator<<(int number) const;

    private:
        bignum_st *m_bn;

        void set_binary(const std::uint8_t *bytes, std::int32_t length, bool little_endian = true);
        void set_dword(std::int32_t value);
        void set_dword(std::uint32_t value);
        void get_bytes(std::uint8_t *buffer, std::size_t length, bool little_endian) const;
        std::int32_t number_of_bytes() const;

        template <typename Container>
        auto set_binary(const Container &container, bool little_endian = true)
            -> std::enable_if_t<!std::is_pointer_v<std::decay_t<Container>>>
        {
            set_binary(std::data(container), std::size(container), little_endian);
        }
    };
} // namespace Crypto
