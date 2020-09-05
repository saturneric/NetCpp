#pragma once

#include <openssl/sha.h>
#include <string>
#include <fstream>
#include <utility>

using std::string;
using std::ifstream;
using std::stringstream;

namespace Net {

/**
 * The Generator of SHA256 Hex
 * We can use it in an easy way.
 */
    class SHA256Generator {
    public:
        SHA256Generator(string data);

        SHA256Generator(ifstream stream);

        void replace(string &str);

        void generate();

        string getHex();

    private:
        bool if_generate = false;
        string raw_data;
        string sha256_data;
    };

}