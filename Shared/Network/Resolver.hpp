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

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <optional>

namespace Network
{
    class Resolver
    {
    public:
        explicit Resolver(boost::asio::io_context &io_context) : m_resolver(io_context) {}

        std::optional<boost::asio::ip::tcp::endpoint> resolve(const boost::asio::ip::tcp &protocol,
                                                              const std::string &host, const std::string &service)
        {
            boost::system::error_code error;
            boost::asio::ip::resolver_base::flags flags = boost::asio::ip::resolver_base::all_matching;
            boost::asio::ip::tcp::resolver::results_type results =
                m_resolver.resolve(protocol, host, service, flags, error);
            if (results.begin() == results.end() || error)
                return {};
            return results.begin()->endpoint();
        }

    private:
        boost::asio::ip::tcp::resolver m_resolver;
    };
} // namespace Network
