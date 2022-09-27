#include <Utilities/Log.hpp>
#include <World/Session.hpp>

namespace World
{
    Session::Session(boost::asio::ip::tcp::socket socket) : Socket(std::move(socket)) {}

    void Session::on_start() { LOG_DEBUG("Connected: {}:{}", remote_address().to_string(), remote_port()); }
} // namespace World
