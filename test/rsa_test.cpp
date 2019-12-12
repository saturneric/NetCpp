//
// Created by Eric Saturn on 2019/12/12.
//

#include <gtest/gtest.h>
#include <rsacpp.h>

#include "env.h"

using namespace Net;
using namespace std;

GlobalTestEnv *_env;

TEST(RSATest, init_test_1) {
    _env->rsa->getDefaultRSAMethod();


}

TEST(RSATest, generate_test_1) {
   _env->rsa->generateKeyPair();
    error::printInfo(to_string(_env->rsa->getBufferSize()), string("Buffer Size"));
}

TEST(RSATest, pub_encrypt_test_1) {
    string encrypted_data;
    _env->rsa->publicKeyEncrypt(_env->rsa_test_data, encrypted_data);
    error::printInfoBuffer(encrypted_data, "Encrypted Data");
    _env->rsa_encrypt_data = encrypted_data;
}

TEST(RSATest, prv_decrypt_test_1){
    string data;
    _env->rsa->privateKeyDecrypt(data, _env->rsa_encrypt_data);
    error::printInfo(data, "Decrypt Data");
}