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
#include <Authentication/Session.hpp>
#include <Authentication/SessionManager.hpp>
#include <Database/AuthDatabase.hpp>
#include <Realm/RealmList.hpp>
#include <Utilities/Log.hpp>
#include <boost/asio/signal_set.hpp>
#include <openssl/provider.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
OSSL_PROVIDER *LegacyProvider;
OSSL_PROVIDER *DefaultProvider;
#endif

int main()
{
    try
    {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        LegacyProvider = OSSL_PROVIDER_load(nullptr, "legacy");
        DefaultProvider = OSSL_PROVIDER_load(nullptr, "default");
#endif

        Utilities::Log::init();

        auto auth_database = Database::AuthDatabase::instance();
        auth_database->open();

        auto io_context = std::make_shared<boost::asio::io_context>();

        auto realm_list = Realm::RealmList::instance();
        realm_list->init(*io_context);

        auto session_manager = Authentication::SessionManager::instance();
        if (!session_manager->init(*io_context, "0.0.0.0", 3724, 1))
        {
            LOG_CRITICAL("Unable to initialize session manager");
            return EXIT_FAILURE;
        }

        boost::asio::signal_set signals(*io_context, SIGINT, SIGTERM);
        signals.async_wait([io_context](const boost::system::error_code &error, int signal) { io_context->stop(); });

        io_context->run();

        signals.cancel();
        auth_database->close();

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        OSSL_PROVIDER_unload(LegacyProvider);
        OSSL_PROVIDER_unload(DefaultProvider);
#endif
    }
    catch (const std::exception &e)
    {
        LOG_CRITICAL("Unhandled standard exception - {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
