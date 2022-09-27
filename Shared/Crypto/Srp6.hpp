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

#include <Crypto/BigNumber.hpp>
#include <Crypto/GenericHash.hpp>
#include <array>
#include <optional>
#include <string>

namespace Crypto
{
    class Srp6
    {
    public:
        static constexpr std::size_t salt_length = 32;
        using Salt = std::array<std::uint8_t, salt_length>;
        static constexpr std::size_t verifier_length = 32;
        using Verifier = std::array<std::uint8_t, verifier_length>;
        static constexpr std::size_t ephemeral_key_length = 32;
        using EphemeralKey = std::array<std::uint8_t, ephemeral_key_length>;
        static constexpr std::size_t session_key_length = 40;
        using SessionKey = std::array<std::uint8_t, session_key_length>;

        static const std::array<std::uint8_t, 1> g;
        static const std::array<std::uint8_t, 32> N;

        static SHA1::Digest session_verifier(const EphemeralKey &p_a, const SHA1::Digest &p_m, const SessionKey &key);

        Srp6(const std::string &username, const Salt &salt, const Verifier &verifier);
        std::optional<SessionKey> verify_challenge(const EphemeralKey &p_a, const SHA1::Digest &p_m);

    private:
        static const BigNumber m_g;
        static const BigNumber m_N;

        const SHA1::Digest m_I;
        const BigNumber m_b;
        const BigNumber m_v;
        bool m_used{false};

        static EphemeralKey calculate_B(const BigNumber &b, const BigNumber &v);
        static Srp6::SessionKey SHA1_inter_leave(const EphemeralKey &S);

    public:
        const Salt s;
        const EphemeralKey B;
    };
} // namespace Crypto
