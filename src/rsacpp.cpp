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

void Net::RSAKeyChain::generateKeyPair() {
    BIGNUM *e = BN_new();
//            生成一个4bit质数
    BN_generate_prime_ex(e, 3, 1, nullptr, nullptr, nullptr);
//            生成一对秘钥
    RSA_generate_key_ex(key_pair, 2048, e, nullptr);
    BN_free(e);
    if(this->key_pair == nullptr) throw runtime_error("key pair generation failed");
    buffer_size = RSA_size(key_pair);
}

void Net::RSAKeyChain::privateKeyDecrypt(string &data, const string &encrypted_data) {
    if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
    assert(buffer_size > 0);
    if(encrypted_data.size() != buffer_size) throw runtime_error("encrypt data's size is abnormal");
//    使用私钥解密
    if(RSA_private_decrypt(encrypted_data.size(), reinterpret_cast<const unsigned char *>(&encrypted_data[0]),
                           reinterpret_cast<unsigned char *>(&data[0]),
                           key_pair,
                           RSA_NO_PADDING) == -1)
        throw  runtime_error(ERR_error_string(ERR_get_error(), nullptr));

}

void Net::RSAKeyChain::publicKeyEncrypt(const string &data, string &encrypted_data) {
    if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
    assert(buffer_size > 0);
    if(data.size() >= this->getBufferSize()) throw runtime_error("string data is too long");
//            预分配储存空间
    encrypted_data.resize(buffer_size);
//            加密数据转移
    string tmp_data = data;
    tmp_data.resize(buffer_size);
//            使用公钥加密
    if(RSA_public_encrypt(tmp_data.size(), reinterpret_cast<const unsigned char *>(&tmp_data[0]),
                          reinterpret_cast<unsigned char *>(&encrypted_data[0]),
                          key_pair,
                          RSA_NO_PADDING) == -1)
        throw  runtime_error(ERR_error_string(ERR_get_error(), nullptr));
}
