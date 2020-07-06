add_library(utils STATIC rsa_key_chain.cpp aes_cbc_encryptor.cpp random_generator.cpp)

target_link_libraries(utils debugTools ssl crypto)