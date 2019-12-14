//
// Created by Eric Saturn on 2019/12/12.
//

#ifndef NET_INIT_H
#define NET_INIT_H

#include "type.h"

class init {
public:
    init() = delete;
    static void doInitWork(){
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
        RSA_meth_get_init();
    }
};


#endif //NET_INIT_H
