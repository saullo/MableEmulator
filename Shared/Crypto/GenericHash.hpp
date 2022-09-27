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
#include <cassert>
#include <openssl/evp.h>
#include <string>
#include <utility>

namespace Crypto
{
    namespace Details
    {
        struct GenericHash
        {
            typedef const EVP_MD *(*HashCreator)();

#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x10100000L
            static EVP_MD_CTX *make_context() noexcept { return EVP_MD_CTX_create(); }
            static void destroy_context(EVP_MD_CTX *ctx) { EVP_MD_CTX_destroy(ctx); }
#else
            static EVP_MD_CTX *make_context() noexcept { return EVP_MD_CTX_new(); }
            static void destroy_context(EVP_MD_CTX *ctx) { EVP_MD_CTX_free(ctx); }
#endif
        };
    }; // namespace Details

    template <Details::GenericHash::HashCreator HashCreator, std::size_t DigestLength> class GenericHash
    {
    public:
        static constexpr size_t disgest_length = DigestLength;
        using Digest = std::array<std::uint8_t, disgest_length>;

        GenericHash() : m_context(Details::GenericHash::make_context())
        {
            auto result = EVP_DigestInit_ex(m_context, HashCreator(), nullptr);
            assert(result == 1);
        }

        GenericHash(const GenericHash &right) : m_context(Details::GenericHash::make_context()) { *this = right; }

        GenericHash(GenericHash &&right) noexcept { *this = std::move(right); }

        ~GenericHash()
        {
            if (!m_context)
                return;
            Details::GenericHash::destroy_context(m_context);
            m_context = nullptr;
        }

        const auto &digest() const { return m_digest; }

        template <typename... T>
        static auto digest_of(T &&...pack) -> std::enable_if_t<!(std::is_integral_v<std::decay_t<T>> || ...), Digest>
        {
            GenericHash hash;
            (hash.update_data(std::forward<T>(pack)), ...);
            hash.finalize();
            return hash.digest();
        }

        static Digest digest_of(const std::uint8_t *data, std::size_t length)
        {
            GenericHash hash;
            hash.update_data(data, length);
            hash.finalize();
            return hash.digest();
        }

        GenericHash &operator=(const GenericHash &right)
        {
            if (this == &right)
                return *this;

            auto result = EVP_MD_CTX_copy_ex(m_context, right.m_context);
            assert(result == 1);
            m_digest = right.m_digest;
            return *this;
        }

        GenericHash &operator=(GenericHash &&right) noexcept
        {
            if (this == &right)
                return *this;

            m_context = std::exchange(right.m_context, Details::GenericHash::make_context());
            m_digest = std::exchange(right.m_digest, Digest{});
            return *this;
        }

    private:
        EVP_MD_CTX *m_context;
        Digest m_digest{};

        void update_data(const std::uint8_t *data, std::size_t length)
        {
            auto result = EVP_DigestUpdate(m_context, data, length);
            assert(result == 1);
        }

        void update_data(std::string_view value)
        {
            update_data(reinterpret_cast<const std::uint8_t *>(value.data()), value.size());
        }

        void update_data(const std::string &value) { update_data(std::string_view(value)); }

        void update_data(const char *value) { update_data(std::string_view(value)); }

        template <typename Container> void update_data(const Container &container)
        {
            update_data(std::data(container), std::size(container));
        }

        void finalize()
        {
            std::uint32_t length;
            auto result = EVP_DigestFinal_ex(m_context, m_digest.data(), &length);
            assert(result == 1);
            assert(length == disgest_length);
        }
    };

    static constexpr std::size_t sha1_digest_length_bytes = 20;

    using SHA1 = GenericHash<EVP_sha1, sha1_digest_length_bytes>;
} // namespace Crypto
