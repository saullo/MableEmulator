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
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <deque>
#include <iostream>

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket)), m_timer(m_socket.get_executor())
    {
        m_timer.expires_at(std::chrono::steady_clock::time_point::max());
    }

    void start()
    {
        auto endpoint = m_socket.remote_endpoint();
        std::cout << "Connected: " << endpoint.address().to_string() << ":" << endpoint.port() << "" << std::endl;

        boost::asio::co_spawn(
            m_socket.get_executor(), [self = this->shared_from_this()] { return self->reader(); },
            boost::asio::detached);

        boost::asio::co_spawn(
            m_socket.get_executor(), [self = this->shared_from_this()] { return self->writer(); },
            boost::asio::detached);
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::steady_timer m_timer;
    std::deque<std::vector<std::uint8_t>> m_queue;

    boost::asio::awaitable<void> reader()
    {
        try
        {
            std::vector<std::uint8_t> buffer;
            buffer.resize(4096);

            while (true)
            {
                auto length =
                    co_await m_socket.async_read_some(boost::asio::buffer(buffer), boost::asio::use_awaitable);
                std::cout << "Message length = " << length << ", data:" << std::endl;
                for (auto byte : buffer)
                    std::cout << byte << std::flush;
                std::cout << std::endl;

                m_queue.push_back(buffer);
                m_timer.cancel_one();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Unhandled reader exception - " << e.what() << std::endl;
            stop();
        }
    }

    boost::asio::awaitable<void> writer()
    {
        try
        {
            while (m_socket.is_open())
            {
                if (m_queue.empty())
                {
                    boost::system::error_code error;
                    co_await m_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, error));
                }
                else
                {
                    auto message = m_queue.front();
                    co_await m_socket.async_write_some(boost::asio::buffer(message), boost::asio::use_awaitable);
                    m_queue.pop_front();
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Unhandled writer exception - " << e.what() << std::endl;
            stop();
        }
    }

    void stop()
    {
        m_socket.close();
        m_timer.cancel();
    }
};

boost::asio::awaitable<void> listener(boost::asio::ip::tcp::acceptor acceptor)
{
    auto endpoint = acceptor.local_endpoint();
    std::cout << "Listening: " << endpoint.address().to_string() << ":" << endpoint.port() << "" << std::endl;

    while (true)
    {
        auto session = std::make_shared<Session>(co_await acceptor.async_accept(boost::asio::use_awaitable));
        session->start();
    }
}

int main()
{
    try
    {
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
        std::cerr << "Unhandled standard exception - " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
