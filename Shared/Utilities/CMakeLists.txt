set(SOURCES
    Log.cpp)

add_library(Utilities ${SOURCES})

find_package(spdlog REQUIRED)
target_link_libraries(Utilities spdlog::spdlog)
