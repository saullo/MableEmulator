set(SOURCES
    Main.cpp
    Session.cpp)

add_executable(AuthenticationServer ${SOURCES})
target_link_libraries(AuthenticationServer Utilities)

find_package(Boost REQUIRED)
target_link_libraries(AuthenticationServer Boost::boost)
