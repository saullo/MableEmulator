set(SOURCES
    Main.cpp
    Session.cpp
    Realm.cpp
    RealmList.cpp)

add_executable(AuthenticationServer ${SOURCES})
target_link_libraries(AuthenticationServer Database Crypto)

find_package(Boost REQUIRED)
target_link_libraries(AuthenticationServer Boost::boost)
