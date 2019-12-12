//
// Created by Eric Saturn on 2019/12/12.
//

#ifndef NET_ENV_H
#define NET_ENV_H

#include "rsacpp.h"

class GlobalTestEnv : public testing::Environment{
public:
    unique_ptr<Net::RSAKeyChain> rsa{new Net::RSAKeyChain()};
};

#endif //NET_ENV_H
