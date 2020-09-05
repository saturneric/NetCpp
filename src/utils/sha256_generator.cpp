#include "type.h"
#include "utils/sha256_generator.h"

/**
 * Generate SHA256 Hex Code
 */
void Net::SHA256Generator::generate() {
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

void Net::SHA256Generator::replace(string &str) {
    this->raw_data = str;
}

string Net::SHA256Generator::getHex() {
    if (!if_generate) generate();
    return this->sha256_data;
}

Net::SHA256Generator::SHA256Generator(ifstream stream) {
    while (stream.good()) {
        stream >> raw_data;
    }
    stream.close();
}

Net::SHA256Generator::SHA256Generator(string data) {
    this->raw_data = std::move(data);
}
