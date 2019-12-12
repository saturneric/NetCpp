//
// Created by Eric Saturn on 2019/12/10.
//

#ifndef NET_RSACPP_H
#define NET_RSACPP_H

#include "error.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>
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

        void generateKeyPair(){
            BIGNUM *e = BN_new();
//            生成一个4bit质数
            BN_generate_prime_ex(e, 3, 1, nullptr, nullptr, nullptr);
//            生成一对秘钥
            RSA_generate_key_ex(key_pair, 2048, e, nullptr);
            BN_free(e);
            if(this->key_pair == nullptr) throw runtime_error("key pair generation failed");
            buffer_size = RSA_size(key_pair);
        }

        void checkKey(){
            if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
            RSA_check_key(this->key_pair);
        }

        void publicKeyEncrypt(string &data, string &encrypted_data){
            if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
            if(data.size() >= this->getBufferSize()) throw runtime_error("string data is too long");
//            预分配储存空间
            encrypted_data.resize(buffer_size);
//            使用公钥加密
            RSA_public_encrypt(data.size(), reinterpret_cast<const unsigned char *>(data.c_str()),
                               reinterpret_cast<unsigned char *>(&data[0]), key_pair, RSA_NO_PADDING);
        }

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
