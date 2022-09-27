#pragma once

#include <Network/Socket.hpp>

namespace World
{
    class Session : public Network::Socket<Session>
    {
    public:
        Session(boost::asio::ip::tcp::socket socket);

    protected:
        void on_start() override;
    };
} // namespace World
