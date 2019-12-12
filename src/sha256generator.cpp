#include "type.h"
#include "sha256generator.h"

void SHA256Generator::generate() {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, raw_data.c_str(), raw_data.size());
    SHA256_Final(hash, &sha256);
    stringstream buffer;
    char buf[2];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i){
        sprintf(buf,"%02x",hash[i]);
        buffer << buf;
    }
    sha256_data = buffer.str();
    if_generate = true;
}
