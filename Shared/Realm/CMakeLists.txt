set(SOURCES
    Realm.cpp
    RealmList.cpp)

add_library(Realm ${SOURCES})
target_link_libraries(Realm Database)
