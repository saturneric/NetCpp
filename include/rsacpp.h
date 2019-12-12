//
// Created by Eric Saturn on 2019/12/10.
//

#ifndef NET_RSACPP_H
#define NET_RSACPP_H

#include "error.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <memory>

using namespace std;

namespace Net {
    class RSAKeyChain {
    public:
        RSAKeyChain() {
            key_pair = RSA_new();
        }

        RSAKeyChain(RSAKeyChain &&t) noexcept {
            this->key_pair = t.key_pair;
            t.key_pair = nullptr;
            this->buffer_size = t.buffer_size;
        }

        void generateKeyPair();

        void checkKey(){
            if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
            RSA_check_key(this->key_pair);
        }

        void publicKeyEncrypt(const string &data, string &encrypted_data);

        void privateKeyDecrypt(string &data, const string& encrypted_data);

        uint32_t getBufferSize() const {
            return this->buffer_size;
        }

        static void getDefaultRSAMethod();

        ~RSAKeyChain() {
            if (key_pair != nullptr) RSA_free(key_pair);
        }

    private:
        RSA *key_pair;
        uint32_t buffer_size = 0;
    };
}


#endif //NET_RSACPP_H
