//
// Created by 胡宇 on 2020/7/7.
//

#include "utils/aes_cbc_encryptor.h"


void Net::AESCBCEncryptor::encrypt(const string &data, string &encrypted_data) {
    int c_len = data.length() + AES_BLOCK_SIZE, f_len = 0;

    auto *encrypt_buffer = reinterpret_cast<uint8_t *>(malloc(c_len));

    EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), nullptr, key, iv);
    EVP_EncryptUpdate(e_ctx, encrypt_buffer, &c_len,
                      reinterpret_cast<const unsigned char *>(data.data()), data.length());
    EVP_EncryptFinal_ex(e_ctx, encrypt_buffer + c_len, &f_len);

    int len = c_len + f_len;

    if(!encrypted_data.empty()) encrypted_data.clear();

    encrypted_data.append(reinterpret_cast<const char *>(encrypt_buffer), len);

    EVP_CIPHER_CTX_reset(e_ctx);
}

void Net::AESCBCEncryptor::decrypt(string &data, const string &encrypt_data) {
    int p_len = encrypt_data.length(), f_len = 0;
    auto *plain_buffer = static_cast<uint8_t *>(malloc(p_len));

    EVP_DecryptInit_ex(e_ctx, EVP_aes_256_cbc(), nullptr, key, iv);
    EVP_DecryptUpdate(e_ctx, plain_buffer, &p_len,
                      reinterpret_cast<const unsigned char *>(encrypt_data.data()), encrypt_data.length());
    EVP_DecryptFinal_ex(e_ctx, plain_buffer + p_len, &f_len);

    int len = p_len + f_len;

    if(!data.empty()) data.clear();

    data.append(reinterpret_cast<const char *>(plain_buffer), len);

    EVP_CIPHER_CTX_reset(e_ctx);
}

void Net::AESCBCEncryptor::generate_random_key_data() {
    Rand::UniformUInt rand(0, UINT32_MAX);

    uint32_t p_data[8];
    for(unsigned int & i : p_data){
        i = rand.generate();
    }

    key_data.append(reinterpret_cast<const char *>(p_data), 32);
}

void Net::AESCBCEncryptor::aes_init(string &key_data) {


    int i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), nullptr,
                           reinterpret_cast<const unsigned char *>(key_data.c_str()), key_data.length(),
                           nrounds, key, iv);
    if (i != 32) {
        throw std::runtime_error("key data must equal 256 bits.");
    }

    EVP_CIPHER_CTX_init(e_ctx);
    EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), nullptr, key, iv);
}
