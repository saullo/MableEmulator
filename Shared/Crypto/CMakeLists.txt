set(SOURCES
    Srp6.cpp
    BigNumber.cpp)

add_library(Crypto ${SOURCES})

find_package(OpenSSL REQUIRED)
target_link_libraries(Crypto OpenSSL::SSL OpenSSL::Crypto)
