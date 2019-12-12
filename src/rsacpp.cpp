//
// Created by Eric Saturn on 2019/12/10.
//

#include "rsacpp.h"

void Net::RSAKeyChain::getDefaultRSAMethod() {
    const RSA_METHOD *rsaMethod = RSA_get_default_method();
    error::printInfoFormal("Default RSA Method", {
        {"name", rsaMethod->name},
    });
}
