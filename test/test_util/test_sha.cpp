//
// Created by 胡宇 on 2020/7/7.
//

#include <gtest/gtest.h>

#include "utils/sha256_generator.h"
#include "debug_tools/print_tools.h"

using namespace Net;

TEST(SHA256Test, base_test_1){
    SHA256Generator generator("Hello World.");
    generator.generate();

    PrintTools::printInfo(generator.getHex(), "SHA256 Hex");

    ASSERT_EQ(generator.getHex(),
            std::string("f4bb1975bf1f81f76ce824f7536c1e101a8060a632a52289d530a6f600d52c92"));
}