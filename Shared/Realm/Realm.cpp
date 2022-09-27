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
#include <Realm/Realm.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Realm
{
    boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> Realm::address_for_client(
        const boost::asio::ip::address &client_address) const
    {
        boost::asio::ip::address realm_ip;
        if (client_address.is_loopback())
        {
            if (local_address.is_loopback() || address.is_loopback())
                realm_ip = client_address;
            else
                realm_ip = local_address;
        }
        else
        {
            auto network = boost::asio::ip::make_network_v4(local_address.to_v4(), local_subnet_mask.to_v4());
            auto hosts = network.hosts();
            auto client_in_network = hosts.find(client_address.to_v4()) != hosts.end();
            if (client_address.is_v4() && client_in_network)
                realm_ip = local_address;
            else
                realm_ip = address;
        }
        return boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>(realm_ip, port);
    }
} // namespace Realm
