//
//  memory.h
//  Net
//
//  Created by 胡一兵 on 2019/1/18.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef memory_h
#define memory_h

#include "type.h"
#include "memory_type.h"

using std::map;
using std::pair;

class BlocksPool{
//    内存块表
    map<void *, block_info> blocks_list;
public:
//    声明某内存块
    void *b_malloc(uint32_t size){
        void *ptr = malloc(size);
        if(ptr == nullptr) return nullptr;
        
        block_info bifo;
        bifo.size = size;
        bifo.lock = 1;
        bifo.pted = false;
        bifo.pvle = ptr;
        
        blocks_list.insert(pair<void *, block_info>(ptr,bifo));
        return ptr;
    }
    
    template<class T>
    void *bv_malloc(T value){
        T *pvalue = (T *)b_malloc(sizeof(T));
        *pvalue = T(value);
        return pvalue;
    }
    
    void *bp_malloc(uint32_t size, void *ptvle){
        void *pvalue = b_malloc(size);
        memcpy(pvalue, ptvle, size);
        return pvalue;
    }
    uint32_t size(void *ptr){
        return blocks_list.find(ptr)->second.size;
    }
//    标记使用某内存块
    void *b_get(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.lock++;
            return ptr;
        }
        else return nullptr;
    }
//    标记保护某内存块
    void b_protect(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.pted = true;
        }
        else throw "protect nil value";
    }
//    标记不再保护某内存块
    void b_noprotect(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.pted = false;
        }
        else throw "noprotect nil value";
    }
//    标记不再使用某内存块
    void b_free(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end() && blk->second.pted == false){
            if(blk->second.lock - 1 == 0){
                free(blk->first);
                blocks_list.erase(blk);
            }
            else blk->second.lock--;
        }
    }
};

extern BlocksPool main_pool;

#endif /* memory_h */
