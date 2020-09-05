//
// Created by Eric Saturn on 2019/12/10.
//

#ifndef NET_RSA_KEY_CHAIN_H
#define NET_RSA_KEY_CHAIN_H

#include "debug_tools/print_tools.h"
#include "../../src/bignumber.cpp"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <memory>
#include <cassert>

using namespace std;

namespace Net {

    /**
     * The Public Key For A RSA Key Chain
     */
    class RSAPubKey{
    public:
        explicit RSAPubKey(const RSA *rsa){
            const BIGNUM *n = nullptr, *e = nullptr, *d = nullptr;
            RSA_get0_key(rsa, &n, &e, &d);

            this->n.copyFrom(n);
            this->e.copyFrom(e);
        }

        void printInfo(){
            PrintTools::printInfoFormal("RSAPubKey Info", {
                    {"n", this->n.getDataHex()},
                    {"e", this->e.getDataHex()}
            });
        }

        BigNumber n;
        BigNumber e;
    };

    /**
     * The Private Key For A RSA Key Chain
     */
    class RSAPrvKey{
    public:
        explicit RSAPrvKey(const RSA *rsa) {
            const BIGNUM *n = nullptr, *e = nullptr, *d = nullptr;
            const BIGNUM *p = nullptr, *q = nullptr;
            RSA_get0_key(rsa, &n, &e, &d);
            this->n.copyFrom(n);
            this->e.copyFrom(e);
            this->d.copyFrom(d);

            RSA_get0_factors(rsa, &p, &q);
            this->p.copyFrom(p);
            this->q.copyFrom(q);
        }

        void printInfo() const{
            PrintTools::printInfoFormal("RSAPrvKey Info", {
                {"n", this->n.getDataHex()},
                {"e", this->e.getDataHex()},
                {"d", this->d.getDataHex()},
                {"p", this->p.getDataHex()},
                {"q", this->q.getDataHex()}
            });
        }

        BigNumber n, e, d;
        BigNumber p, q;
    };

    class RSAKeyChain {
    public:
        RSAKeyChain() {
            key_pair = RSA_new();
        }

        RSAKeyChain(RSAKeyChain &&t) noexcept {
            this->key_pair = t.key_pair;
            t.key_pair = nullptr;
            this->buffer_size = t.buffer_size;
            this->if_prv_key = t.if_prv_key;
            this->if_pub_key = t.if_pub_key;
        }

        explicit RSAKeyChain(const RSAPubKey& pubKey){
            key_pair = RSA_new();
            RSA_set0_key(key_pair, pubKey.n.getCopy(),  pubKey.e.getCopy(), nullptr);
            this->if_pub_key = true;
        }

        explicit RSAKeyChain(const RSAPrvKey& prvKey){
            key_pair = RSA_new();
            RSA_set0_key(this->key_pair, prvKey.n.getCopy(), prvKey.e.getCopy(), prvKey.d.getCopy());
            RSA_set0_factors(key_pair, prvKey.p.getCopy(), prvKey.q.getCopy());
            this->if_prv_key = true;
        }

//        生成一对公私钥
        void generateKeyPair();

//        检查私钥是否合法
        bool checkKey();

//        公钥加密
        void publicKeyEncrypt(const string &data, string &encrypted_data);

//        私钥解密
        void privateKeyDecrypt(string &data, const string& encrypted_data);

        uint32_t getBufferSize() const {
            return this->buffer_size;
        }

        const RSA *getRSA(){
            return key_pair;
        }

        RSAPubKey getPubKey(){
            RSAPubKey pubKey(key_pair);
            return pubKey;
        }

        RSAPrvKey getPrvKey(){
            RSAPrvKey prvKey(key_pair);
            return prvKey;
        }

        ~RSAKeyChain() {
            if (key_pair != nullptr) RSA_free(key_pair);
        }

    private:
        RSA *key_pair{};
        uint32_t buffer_size = 0;
        bool if_prv_key = false, if_pub_key = false;
    };
}


#endif //NET_RSA_KEY_CHAIN_H
