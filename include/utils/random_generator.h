//
// Created by 胡宇 on 2020/7/7.
//

#ifndef NET_RANDOM_GENERATOR_H
#define NET_RANDOM_GENERATOR_H

#include <boost/random.hpp>
#include <boost/random/random_device.hpp>

namespace Net{
    namespace Rand{

//        范围均匀分布无符号32位整数
        class UniformUInt {
        public:
            UniformUInt(uint32_t min, uint32_t max) : uniformInt(min, max){

            }

            int generate() const;
        private:
            boost::uniform_int<uint32_t> uniformInt;
        };

    }
}


#endif //NET_RANDOM_GENERATOR_H
