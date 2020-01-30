//
// Created by Eric Saturn on 2019/12/12.
//

#include <gtest/gtest.h>
#include <rsa_cpp_binding.h>

#include "env.h"

using namespace Net;
using namespace std;

extern GlobalTestEnv *_env;

TEST(RSATest, init_test_1) {

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
    ASSERT_EQ(data, _env->rsa_test_data);
}

TEST(RSATest, pub_key_get_test_1){
    _env->pubKey = shared_ptr<RSAPubKey>(new RSAPubKey(_env->rsa->getRSA()));

}

TEST(RSATest, prv_key_get_test_1){
    _env->prvKey = shared_ptr<RSAPrvKey>(new RSAPrvKey(_env->rsa->getRSA()));
    ASSERT_EQ(_env->rsa->checkKey(), true);
    _env->prvKey->printInfo();
}

TEST(RSATest, prv_key_build_key_chain){
    RSAKeyChain keyChain(*_env->prvKey);
    ASSERT_EQ(keyChain.checkKey(), true);
}