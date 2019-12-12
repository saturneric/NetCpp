//
// Created by Eric Saturn on 2019/12/12.
//

#include <gtest/gtest.h>

#include "env.h"

extern GlobalTestEnv *_env;

int main(int argc, char *argv[]){
    ::testing::InitGoogleTest(&argc, argv);
    _env = dynamic_cast<GlobalTestEnv *>(::testing::AddGlobalTestEnvironment(new GlobalTestEnv));
    return RUN_ALL_TESTS();
}
