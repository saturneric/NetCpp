add_library(utils STATIC rsa_key_chain.cpp)

target_link_libraries(utils debugTools ssl crypto)