set(SOURCES
    Main.cpp
    Session.cpp
    SessionManager.cpp)

add_executable(AuthenticationServer ${SOURCES})
target_link_libraries(AuthenticationServer Database Crypto Realm)

find_package(Boost REQUIRED)
target_link_libraries(AuthenticationServer Boost::boost)
