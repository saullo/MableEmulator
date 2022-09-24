set(SOURCES
    Main.cpp)

add_executable(AuthenticationServer ${SOURCES})

find_package(Boost REQUIRED)
target_link_libraries(AuthenticationServer Boost::boost)

find_package(spdlog REQUIRED)
target_link_libraries(AuthenticationServer spdlog::spdlog)
