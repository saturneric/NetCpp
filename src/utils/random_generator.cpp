//
// Created by 胡宇 on 2020/7/7.
//

#include "utils/random_generator.h"

boost::random::mt19937 rand_seed;

int Net::Rand::UniformUInt::generate() const {
    return uniformInt(rand_seed);
}
