//
//  memory_type.h
//  Net
//
//  Created by 胡一兵 on 2019/1/19.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef memory_type_h
#define memory_type_h

#include <stdint.h>

//内存块管理类
class block_info{
    friend class BlocksPool;
//    内存块大小
    uint32_t size = 0;
//    锁
    int lock = 0;
//    是否受到保护
    bool pted = false;
//    指向内存块的指针
    void *pvle = nullptr;
    
    block_info(){
        
    }
public:
//    简化构造函数
    block_info(uint32_t size,void *pvle){
        this->size = size;
        this->pvle = pvle;
    }
    void *get_pvle(void){
        return pvle;
    }
    
    uint32_t get_size(void){
        return size;
    }
    
};

#endif /* memory_type_h */
