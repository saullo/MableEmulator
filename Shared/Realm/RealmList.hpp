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

#include <Network/Resolver.hpp>
#include <Realm/Realm.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <map>
#include <vector>

namespace Realm
{
    class RealmList
    {
    public:
        struct BuildInformation
        {
        public:
            std::uint32_t build;
            std::uint32_t major;
            std::uint32_t minor;
            std::uint32_t revision;
        };

        static RealmList *instance();

        const auto &realms() const { return m_realms; }

        void init(boost::asio::io_context &io_context);
        const BuildInformation *build_info(std::uint32_t build) const;

        bool is_pre_bc_client(std::uint32_t build);
        bool is_post_bc_client(std::uint32_t build);

    private:
        using DeadlineTimer = boost::asio::basic_deadline_timer<boost::posix_time::ptime,
                                                                boost::asio::time_traits<boost::posix_time::ptime>,
                                                                boost::asio::io_context::executor_type>;

        static constexpr auto max_pre_bc_client_build = 6141;
        static RealmList *m_instance;

        std::vector<BuildInformation> m_builds;
        std::map<std::uint32_t, Realm> m_realms;
        std::unique_ptr<Network::Resolver> m_resolver;
        std::unique_ptr<DeadlineTimer> m_timer;

        void init_builds();
        void update_realms(boost::system::error_code error);
    };
} // namespace Realm
