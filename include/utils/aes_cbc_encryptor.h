//
// Created by 胡宇 on 2020/7/7.
//

#ifndef NET_AES_CBC_ENCRYPTOR_H
#define NET_AES_CBC_ENCRYPTOR_H

#include <cassert>
#include <cstdint>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <string>

#include "debug_tools/print_tools.h"
#include "random_generator.h"

namespace Net {

class AESCBCEncryptor {
public:
  AESCBCEncryptor();

  string getKeyData() const;

  void encrypt(const std::string &data, std::string &encrypted_data);

  void decrypt(std::string &data, const std::string &encrypt_data);

private:
  const int nrounds = 8;

  uint8_t key[32], iv[32];

  EVP_CIPHER_CTX *e_ctx = EVP_CIPHER_CTX_new();

  std::string key_data;

  void generate_random_key_data();

  void aes_init(std::string &key_data);
};

} // namespace Net

#endif // NET_AES_CBC_ENCRYPTOR_H
