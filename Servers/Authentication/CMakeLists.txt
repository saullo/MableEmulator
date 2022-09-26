set(SOURCES
    Main.cpp
    Session.cpp)

add_executable(AuthenticationServer ${SOURCES})
target_link_libraries(AuthenticationServer Database)

find_package(Boost REQUIRED)
target_link_libraries(AuthenticationServer Boost::boost)
