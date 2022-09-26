#pragma once

#include <Database/Connection.hpp>

namespace Database
{
    class AuthDatabase : public Connection
    {
    public:
        AuthDatabase();

        static AuthDatabase *instance();

    private:
        static AuthDatabase *m_instance;
    };
} // namespace Database
