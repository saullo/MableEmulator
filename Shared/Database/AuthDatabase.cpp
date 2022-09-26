#include <Database/AuthDatabase.hpp>

namespace Database
{
    AuthDatabase *AuthDatabase::m_instance = nullptr;

    AuthDatabase::AuthDatabase() : Connection("127.0.0.1", 3306, "root", "root", "auth") {}

    AuthDatabase *AuthDatabase::instance()
    {
        if (!m_instance)
            m_instance = new AuthDatabase();
        return m_instance;
    }

} // namespace Database
