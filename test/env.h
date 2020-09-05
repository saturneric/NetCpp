//
// Created by Eric Saturn on 2019/12/12.
//

#ifndef NET_ENV_H
#define NET_ENV_H

#include "utils/rsa_key_chain.h"

class GlobalTestEnv : public testing::Environment{
public:
    unique_ptr<Net::RSAKeyChain> rsa{new Net::RSAKeyChain()};
    string rsa_test_data = "hello world";
    string rsa_encrypt_data;
    shared_ptr<Net::RSAPrvKey> prvKey = nullptr;
    shared_ptr<Net::RSAPubKey> pubKey = nullptr;
};

#endif //NET_ENV_H
