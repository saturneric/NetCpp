//
// Created by Eric Saturn on 2019/12/10.
//

#include "rsa_cpp_binding.h"

void Net::RSAKeyChain::generateKeyPair() {
    BigNumber e;
//            生成一个4bit质数
    BN_generate_prime_ex(e.get(), 3, 1, nullptr, nullptr, nullptr);
//            生成一对秘钥
    RSA_generate_key_ex(key_pair, 2048, e.get(), nullptr);
    if(this->key_pair == nullptr) throw runtime_error("key pair generation failed");
    buffer_size = RSA_size(key_pair);
    this->if_prv_key = true;
    this->if_pub_key = true;
}

void Net::RSAKeyChain::privateKeyDecrypt(string &data, const string &encrypted_data) {
    if(!this->if_prv_key) throw runtime_error("illegal call of privateKeyDecrypt");
    if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
    assert(buffer_size > 0);
    if(encrypted_data.size() != buffer_size) throw runtime_error("encrypt data's size is abnormal");
//    使用私钥解密
    int decrypted_size = -1;
    unique_ptr<unsigned char[]>p_buffer (new unsigned char[buffer_size]);
    if((decrypted_size = RSA_private_decrypt(encrypted_data.size(),
            reinterpret_cast<const unsigned char *>(&encrypted_data[0]),
            p_buffer.get(),
            key_pair,
            RSA_PKCS1_OAEP_PADDING)) == -1)
        throw  runtime_error(ERR_error_string(ERR_get_error(), nullptr));
    else data = string(reinterpret_cast<const char *>(p_buffer.get()));
}

void Net::RSAKeyChain::publicKeyEncrypt(const string &data, string &encrypted_data) {
    if(!this->if_pub_key) throw runtime_error("illegal call of publicKeyEncrypt");
    if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
    assert(buffer_size > 0);
    if(data.size() >= buffer_size - 42) throw runtime_error("string data is too long");
//            预分配储存空间
    encrypted_data.resize(buffer_size);
//            加密数据转移
    string tmp_data = data;
    tmp_data.resize(buffer_size - 42);
//            使用公钥加密
    int encrypted_size = -1;
    if((encrypted_size = RSA_public_encrypt(tmp_data.size(),
            reinterpret_cast<const unsigned char *>(&tmp_data[0]),
            reinterpret_cast<unsigned char *>(&encrypted_data[0]),
            key_pair,
            RSA_PKCS1_OAEP_PADDING)) == -1)
        throw  runtime_error(ERR_error_string(ERR_get_error(), nullptr));
}

bool Net::RSAKeyChain::checkKey() {
    if(!this->if_prv_key) throw runtime_error("illegal call of checkKey");
    if(this->key_pair == nullptr) throw runtime_error("key pair is invalid");
    int return_code = RSA_check_key(this->key_pair);
    if(return_code == -1) throw runtime_error("error occur when rsa check key");
    else return return_code == 1;
}
