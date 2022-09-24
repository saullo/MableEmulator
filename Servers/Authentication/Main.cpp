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
#include <Authentication/Log.hpp>
#include <Authentication/Session.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>

boost::asio::awaitable<void> listener(boost::asio::ip::tcp::acceptor acceptor)
{
    auto endpoint = acceptor.local_endpoint();
    LOG_INFO("Listening: {}:{}", endpoint.address().to_string(), endpoint.port());

    while (true)
    {
        auto session =
            std::make_shared<Authentication::Session>(co_await acceptor.async_accept(boost::asio::use_awaitable));
        session->start();
    }
}

int main()
{
    try
    {
        spdlog::set_level(spdlog::level::debug);
        boost::asio::io_context io_context(1);
        boost::asio::co_spawn(io_context,
                              listener(boost::asio::ip::tcp::acceptor(io_context, {boost::asio::ip::tcp::v4(), 3724})),
                              boost::asio::detached);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });
        io_context.run();
    }
    catch (const std::exception &e)
    {
        LOG_CRITICAL("Unhandled standard exception - {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
