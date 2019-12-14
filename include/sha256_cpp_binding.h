#pragma once

#include <openssl/sha.h>
#include <string>
#include <fstream>
#include <utility>

using std::string;
using std::ifstream;
using std::stringstream;

class SHA256Generator{
public:
    SHA256Generator(string data){
        this->raw_data = std::move(data);
    }

    SHA256Generator(ifstream stream){
        while(stream.good()){
            stream >> raw_data;
        }
        stream.close();
    }

    void generate();

    string getHex(){
        if(!if_generate) generate();
        return this->sha256_data;
    }
private:
    bool if_generate = false;
    string raw_data;
    string sha256_data;
};
