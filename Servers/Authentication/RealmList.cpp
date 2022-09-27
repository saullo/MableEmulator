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
#include <Authentication/RealmList.hpp>
#include <Database/AuthDatabase.hpp>
#include <Utilities/Log.hpp>

namespace Authentication
{
    RealmList *RealmList::m_instance = nullptr;

    RealmList *RealmList::instance()
    {
        if (!m_instance)
            m_instance = new RealmList();
        return m_instance;
    }

    void RealmList::init(boost::asio::io_context &io_context)
    {
        m_resolver = std::make_unique<Network::Resolver>(io_context);
        m_timer = std::make_unique<DeadlineTimer>(io_context);

        init_builds();
        boost::system::error_code error;
        update_realms(error);
    }

    const RealmList::BuildInformation *RealmList::build_info(std::uint32_t build_number) const
    {
        for (const auto &build : m_builds)
        {
            if (build.build == build_number)
                return &build;
        }
        return nullptr;
    }

    void RealmList::init_builds()
    {
        if (auto query = Database::AuthDatabase::instance()->query(
                "SELECT build, major, minor, revision FROM build_information"))
        {
            do
            {
                auto fields = query->fetch();
                auto &build = m_builds.emplace_back();
                build.build = fields[0].get_uint32();
                build.major = fields[1].get_uint32();
                build.minor = fields[2].get_uint32();
                build.revision = fields[3].get_uint32();
            } while (query->next_row());
        }
    }

    void RealmList::update_realms(boost::system::error_code error)
    {
        if (error)
            return;

        if (auto query =
                Database::AuthDatabase::instance()->query("SELECT id, name, address, local_address, local_subnet_mask, "
                                                          "port, type, flags, category, population, build FROM "
                                                          "realmlist WHERE flags <> 3"))
        {
            do
            {
                auto fields = query->fetch();
                auto id = fields[0].get_uint32();
                auto name = fields[1].get_string();

                auto address_string = fields[2].get_string();
                auto address = m_resolver->resolve(boost::asio::ip::tcp::v4(), address_string, "");
                if (!address)
                {
                    LOG_ERROR("Failed to resolve address = {}, realm = {}, id = {}", address_string.c_str(),
                              name.c_str(), id);
                    continue;
                }

                auto local_address_string = fields[3].get_string();
                auto local_address = m_resolver->resolve(boost::asio::ip::tcp::v4(), local_address_string, "");
                if (!local_address)
                {
                    LOG_ERROR("Failed to resolve local address = {}, realm = {}, id = {}", local_address_string.c_str(),
                              name.c_str(), id);
                    continue;
                }

                auto local_subnet_string = fields[4].get_string();
                auto local_submask_address = m_resolver->resolve(boost::asio::ip::tcp::v4(), local_subnet_string, "");
                if (!local_submask_address)
                {
                    LOG_ERROR("Failed to resolve local subnet mask = {}, realm = {}, id = {}",
                              local_subnet_string.c_str(), name.c_str(), id);
                    continue;
                }

                auto port = fields[5].get_uint16();
                auto type = fields[6].get_uint8();
                if (type == realmtype_ffa_pvp)
                    type = realmtype_pvp;
                if (type >= realmtype_max_client)
                    type = realmtype_normal;
                auto flags = fields[7].get_uint8();
                auto category = fields[8].get_uint8();
                auto population = fields[9].get_float();
                auto build = fields[10].get_uint32();

                if (!m_realms.contains(id))
                {
                    LOG_DEBUG("Added realm id = {}, name = {}, type = {}, flags = {}, population = {}, category = {}",
                              id, name, type, flags, population, category);
                }
                else
                {
                    LOG_DEBUG("Updated realm id = {}, name = {}, type = {}, flags = {}, population = {}, category = {}",
                              id, name, type, flags, population, category);
                }

                auto &realm = m_realms[id];
                realm.id = id;
                realm.name = name;
                realm.address = address->address();
                realm.local_address = local_address->address();
                realm.local_subnet_mask = local_submask_address->address();
                realm.port = port;
                realm.type = type;
                realm.flags = RealmFlags(flags);
                realm.category = category;
                realm.population = population;
                realm.build = build;
            } while (query->next_row());
        }

        m_timer->expires_from_now(boost::posix_time::seconds(30));
        m_timer->async_wait([this](auto code) { update_realms(code); });
    }

    bool RealmList::is_pre_bc_client(std::uint32_t build)
    {
        return build <= max_pre_bc_client_build && build_info(build);
    }

    bool RealmList::is_post_bc_client(std::uint32_t build)
    {
        return build > max_pre_bc_client_build && build_info(build);
    }

} // namespace Authentication
