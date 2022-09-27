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

#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace Realm
{
    enum RealmType
    {
        realmtype_normal = 0,
        realmtype_pvp = 1,
        realmtype_max_client = 14,
        realmtype_ffa_pvp = 16
    };

    enum RealmFlags
    {
        realmflag_none = 0x00,
        realmflag_version_mismatch = 0x01,
        realmflag_offline = 0x02,
        realmflag_specifybuild = 0x04,
        realmflag_unknown1 = 0x08,
        realmflag_unknown2 = 0x10,
        realmflag_recommended = 0x20,
        realmflag_new = 0x40,
        realmflag_full = 0x80
    };

    struct Realm
    {
        std::uint32_t id;
        std::string name;
        boost::asio::ip::address address;
        boost::asio::ip::address local_address;
        boost::asio::ip::address local_subnet_mask;
        std::uint16_t port;
        std::uint8_t type;
        RealmFlags flags;
        std::uint8_t category;
        float population;
        std::uint32_t build;

        boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> address_for_client(
            const boost::asio::ip::address &client_address) const;
    };
} // namespace Realm
