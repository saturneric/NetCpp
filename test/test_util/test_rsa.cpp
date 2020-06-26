//
// Created by Eric Saturn on 2019/12/12.
//

#include <gtest/gtest.h>

#include <memory>

#include "../env.h"

#include "utils/rsa_key_chain.h"

using namespace Net;
using namespace std;

extern GlobalTestEnv *env;

TEST(RSATest, init_test_1) {

}

TEST(RSATest, generate_test_1) {
   env->rsa->generateKeyPair();
   printTools::printInfo(to_string(env->rsa->getBufferSize()), string("Buffer Size"));
}

TEST(RSATest, pub_encrypt_test_1) {
    string encrypted_data;
    env->rsa->publicKeyEncrypt(env->rsa_test_data, encrypted_data);
    printTools::printInfoBuffer(encrypted_data, "Encrypted Data");
    env->rsa_encrypt_data = encrypted_data;
}

TEST(RSATest, prv_decrypt_test_1){
    string data;
    env->rsa->privateKeyDecrypt(data, env->rsa_encrypt_data);
    printTools::printInfo(data, "Decrypt Data");
    ASSERT_EQ(data, env->rsa_test_data);
}

TEST(RSATest, pub_key_get_test_1){
    env->pubKey = std::make_shared<RSAPubKey>(env->rsa->getRSA());

}

TEST(RSATest, prv_key_get_test_1){
    env->prvKey = std::make_shared<RSAPrvKey>(env->rsa->getRSA());
    ASSERT_EQ(env->rsa->checkKey(), true);
    env->prvKey->printInfo();
}

TEST(RSATest, prv_key_build_key_chain){
    RSAKeyChain keyChain(*env->prvKey);
    ASSERT_EQ(keyChain.checkKey(), true);
}