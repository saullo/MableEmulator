set(SOURCES
    Connection.cpp
    AuthDatabase.cpp
    ResultSet.cpp
    Field.cpp)

add_library(Database ${SOURCES})
target_link_libraries(Database Utilities)

find_package(MySQL REQUIRED)
target_link_libraries(Database ${MYSQL_LIBRARY})
target_include_directories(Database PUBLIC ${MYSQL_INCLUDE_DIR})
