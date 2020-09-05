//
// Created by 胡宇 on 2020/7/7.
//

#include <gtest/gtest.h>

#include "utils/aes_cbc_encryptor.h"

using namespace Net;

TEST(AES_Test, base_test_1){
    AESCBCEncryptor encryptor;

    PrintTools::printInfoBuffer(encryptor.getKeyData(), "Key Data");

    std::string data, encrypt_data;

    encryptor.encrypt("Hello World", encrypt_data);

    PrintTools::printInfoBuffer(encrypt_data, "Encrypt Data");

    encryptor.decrypt(data, encrypt_data);

    ASSERT_EQ(data, std::string("Hello World"));
}