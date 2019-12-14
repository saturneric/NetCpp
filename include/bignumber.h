//
// Created by Eric Saturn on 2019/12/13.
//

#ifndef NET_BIGNUMBER_H
#define NET_BIGNUMBER_H

#include <memory>
#include <openssl/bn.h>

using namespace std;

namespace Net {
//    对BIGNUM进行简单封装
    class BigNumber {
    public:
        BigNumber() : bn(BN_new(), ::BN_free) {}

        BigNumber(BIGNUM *t_bn) : bn(t_bn, ::BN_free) {}

//        临时取用
        BIGNUM * get() const { return bn.get(); }

        BIGNUM *getCopy() const {
            BIGNUM *n_bn = BN_new();
            BN_copy(n_bn, bn.get());
            return n_bn;
        }

//        获得智能指针
        shared_ptr<BIGNUM> getSharedPtr(){
            return bn;
        }

//        获得一份拷贝
        BigNumber copy(){
            BigNumber n;
            BN_copy(n.get(), bn.get());
            return n;
        }

        void copyFrom(const BIGNUM * t){
            BN_copy(bn.get(), t);
        }

//        向该类移交BIGNUM结构的的控制权
        void set(BIGNUM *t_bn) { bn = shared_ptr<BIGNUM>(t_bn, ::BN_free); }

//        该类移交所管辖的BIGNUM结构的控制权
        BIGNUM *getControl() {
            if(bn != nullptr){
                BIGNUM *n_bn = bn.get();
                bn = nullptr;
                return n_bn;
            }
            else return nullptr;
        }

//        得到BIGNUM的Hex字符串
        string getDataHex() const {
            void *hex_data_str = BN_bn2hex(bn.get());
            string hex_string((const char *)hex_data_str);
            OPENSSL_free(hex_data_str);
            return hex_string;
        }

        string getDataHexHash() const{
            return string();
        }

    private:
        shared_ptr<BIGNUM> bn;
    };
}


#endif //NET_BIGNUMBER_H
