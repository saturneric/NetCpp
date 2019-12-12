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
    error::printInfo(to_string(_env->rsa->getBufferSize()), string("Buffer Size"));

}

TEST(RSATest, generate_test_1) {
   _env->rsa->generateKeyPair();
}

TEST(RSATest, pub_encrypt_test_1) {
    RSAKeyChain rsa;
    rsa.generateKeyPair();
}