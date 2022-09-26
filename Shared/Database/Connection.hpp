#pragma once

#include <Database/ResultSet.hpp>
#include <cstdint>
#include <mysql/mysql.h>

namespace Database
{
    class Connection
    {
    public:
        Connection(const char *host, int port, const char *user, const char *password, const char *database);

        std::uint32_t open();
        void close();
        ResultSet *query(const char *sql);

    private:
        MYSQL *m_handler;
        int m_port{-1};
        const char *m_host{nullptr};
        const char *m_user{nullptr};
        const char *m_password{nullptr};
        const char *m_database{nullptr};
    };
} // namespace Database
