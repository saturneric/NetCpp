#pragma once

#include <openssl/sha.h>
#include <string>
#include <vector>
#include <fstream>
#include <utility>

using std::string;
using std::ifstream;
using std::stringstream;
using std::vector;

namespace Net {

/**
 * The Generator of SHA256 Hex
 * We can use it in an easy way.
 */
    class SHA256Generator {
    public:
        SHA256Generator();

        void setRawData(const string &str);

        void setRawData(const vector<char> &c_array);

        void generate();

        string getHex();

    private:
        bool if_generate = false;
        const void *raw_data = nullptr;
        size_t raw_data_size = 0;
        string sha256_data;
    };

}
