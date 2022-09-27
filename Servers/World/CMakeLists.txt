set(SOURCES
    Main.cpp
    Session.cpp)

add_executable(WorldServer ${SOURCES})
target_link_libraries(WorldServer Database Crypto)
