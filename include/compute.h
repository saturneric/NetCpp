//
//  compute.h
//  Net
//
//  Created by 胡一兵 on 2019/1/19.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef compute_h
#define compute_h

#include "memory_type.h"
#include <vector>
using std::vector;

//声明计算模块的传入与传出参数列表
#define ARGS_DECLAER(name) vector<block_info> __##name##_args_in, __##name##_args_out
//声明计算模块的入口
#define PCSFUNC_DEFINE(name) extern "C" int name(void)

//合并计算模块入口函数定义及参数入口与出口操作柄声明
#define _PCSFUNC(name) ARGS_DECLAER(name); \
PCSFUNC_DEFINE(name)

//用户获得计算过程入口操作柄
#define ARGS_IN(name) __##name##_args_in
//用户获得计算过程出口操作柄
#define ARGS_OUT(name) __##name##_args_out

//调用成功的返回
#define SUCCESS 0
//调用失败的返回
#define FAIL -1

//输入或输出入口操作柄管理类
class LibArgsTransfer{
    friend class CPart;
//    计算过程参数入口或出口操作柄
    vector<block_info> *args = nullptr;
//    计算模块向入口添加自定义指针参数
    void addArgPtr(int size, void *p_arg);
//    清空参数入口与出口
    void clear(void);
//    空构造函数
    LibArgsTransfer();
public:
//    计算过程构造该类的唯一构造函数
    LibArgsTransfer(vector<block_info> &args){
        this->args = &args;
    }
//    计算过程从入口处获得参数
    template<class T>
    T get_arg(int idx){
        T *pvle = (T *)(*args)[idx].get_pvle();
//        检查用户所提供的类型与入口该位置的参数大小是否匹配
        if((*args)[idx].get_size() == sizeof(T)) return *pvle;
        else throw "arg size conflict";
    }
//    计算过程从入口获得自定义指针参数
    template<class T>
    T *get_arg_ptr(int idx){
        T *pvle = (*args)[idx].get_pvle();
        return pvle;
    }
//    计算过程向出口末尾添加自定义指针参数
    void push_arg_ptr(int size, void *p_arg){
        block_info pbifo(size,p_arg);
        args->push_back(pbifo);
    }
//    计算过程向出口末尾添加参数
    template<class T>
    void push_arg(T arg){
        T *p_arg = (T *)malloc(sizeof(T));
        *p_arg = arg;
        block_info pbifo(sizeof(T),p_arg);
        args->push_back(pbifo);
    }
    ~LibArgsTransfer(){
        //clear();
    }
};

#endif /* compute_h */
