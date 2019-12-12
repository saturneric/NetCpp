//
//  cpart.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "memory.h"
#include "cpart.h"


/**
 计算模块类构造函数

 @param t_srcpath 源文件路径
 @param t_libpath 生成的动态链接库路径
 @param t_srcname 源文件名
 @param t_name 模块入口函数名
 @param ffresh 是否强制刷新动态链接库
 */
CPart::CPart(string t_srcpath,string t_libpath,string t_srcname,string t_name,bool ffresh):func(nullptr),handle(nullptr){
    
    this->name = t_name;

//    修饰源文件路径名
    t_srcpath.erase(t_srcpath.find_last_not_of(" ")+1);
    if(t_srcpath.back() == '/') t_srcpath.pop_back();
    this->src_path = t_srcpath;
    
//    除去源文件名两端的空格
    t_srcname.erase(t_srcname.find_last_not_of(" ")+1,t_srcname.length());
    t_srcname.erase(0,t_srcname.find_first_not_of(" "));
    
//    检查源文件名是否合法
    unsigned long qpidx = t_srcname.find(".cpp",0);
    if(qpidx == string::npos) throw "source file's name illegal(.cpp)";
    for (auto c : t_srcname.substr(0,qpidx)) {
        if(isalnum(c) || c == '_');
        else throw "source file's name has illegal char";
    }
    
    //    检查源文件文件是否存在且可写
    if(access((src_path+"/"+t_srcname).data(), W_OK) == -1) throw "source file's state abnormal";
    this->src_name = t_srcname;
    
    
//    根据合法的源文件名生成lib文件的文件名
    string t_libname = "lib"+src_name.substr(0,qpidx)+".so";
    this->lib_name = t_libname;
    
//    修饰动态链接库路径名
    t_libpath.erase(t_libpath.find_last_not_of(" ")+1);
    if(t_libpath.back() == '/') t_libpath.pop_back();
    
//    检查动态链接库生成目录是否存在且可写
    if(access((t_libpath).data(), W_OK) == -1) throw "library path is abnormal";
    this->lib_path = t_libpath;
    
//    检查模块入口函数名是否含有非法字符
    for (auto c : t_name) {
        if(isalnum(c) || c == '_');
        else throw "PCSFUNC's name has illegal char";
    }
    this->name = t_name;
    
//    如果lib文件存在且不要求每次建立该结构都重新编译一次源文件的话就不执行编译
    if(!~access((lib_path+lib_name).data(), F_OK) || ffresh){
//        编译源文件生成动态链接库
        BuildSo();
//        获得动态链接库的操作柄
        GetSoHandle();
    }

}

CPart::CPart(string func_name, sqlite3 *psql){
//    获得动态链接库名
    string sql_quote = "SELECT libfiles.name FROM functions INNER JOIN libfiles ON libfiles.id = functions.libfile_id WHERE functions.name = ?1;";
    sqlite3_stmt *psqlsmt;
    const char *pzTail;
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_bind_text(psqlsmt, 1, func_name.data(), -1, SQLITE_TRANSIENT);
    sqlite3_step(psqlsmt);
    lib_name = (char *)sqlite3_column_text(psqlsmt, 0);
    sqlite3_finalize(psqlsmt);
//    获得工程信息
    sql_quote = "SELECT * FROM projfile;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    lib_path = (char *)sqlite3_column_text(psqlsmt, 3);
    name = (char *)sqlite3_column_text(psqlsmt, 0);
    sqlite3_finalize(psqlsmt);
//    获得相关操作柄
    GetSoHandle();
//    记录形式参数
    sql_quote = "SELECT * FROM fargs_"+func_name+" WHERE io = ?1;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_bind_int(psqlsmt, 1, 0);
    int rtn = sqlite3_step(psqlsmt);
    if(rtn != SQLITE_DONE)
        do{
            farg_info nfi;
            nfi.type =(char *) sqlite3_column_text(psqlsmt, 1);
            nfi.size =sqlite3_column_int(psqlsmt, 5);
            fargs_in.push_back(nfi);
            
        }while (sqlite3_step(psqlsmt) != SQLITE_DONE);
    sqlite3_reset(psqlsmt);
    sqlite3_clear_bindings(psqlsmt);
    sqlite3_bind_int(psqlsmt, 1, 1);
    if(rtn != SQLITE_DONE)
        do{
            farg_info nfi;
            nfi.type =(char *) sqlite3_column_text(psqlsmt, 1);
            nfi.size =sqlite3_column_int(psqlsmt, 5);
            fargs_out.push_back(nfi);
        }while (sqlite3_step(psqlsmt) != SQLITE_DONE);
    sqlite3_finalize(psqlsmt);
}


/**
 执行源文件编译操作，在指定位置生成动态链接库

 @return 执行成功将返回0
 */
int CPart::BuildSo(void){
    int rtn = system(("g++ -fPIC -shared -std=c++11 -o "+lib_path+"/"+lib_name+" "+src_path+"/"+src_name).data());
//    检测命令执行情况
    if(rtn != -1 && rtn != 127)
        return 0;
    else throw "fail to build lib file";
}


/**
 获得动态链接库的操作柄，并获得传入参数的操作柄及传出参数的操作柄。

 @return 执行成功则返回0
 */
int CPart::GetSoHandle(void){
//    检查动态链接库生成目录是否存在且可读
    if(access((lib_path).data(), R_OK) == -1) throw "library not found";
//    获得动态链接库的操作柄
    this->handle = dlopen((lib_path+"/"+lib_name).data(), RTLD_NOW | RTLD_LOCAL);
    if(this->handle == nullptr) throw "can not open library";
//    获得该模块的入口
    this->func = (PCSFUNC) dlsym(this->handle, this->name.data());
    if(this->func == nullptr) throw "can not get func "+this->name;
//    获得向该模块传入参数的操作柄
    this->libargs_in.args = (vector<block_info> *) dlsym(this->handle, ("__"+name+"_args_in").data());
    if(this->libargs_in.args == nullptr) throw "can not get the HANDLE to PUSH args";
//    获得获取该模块传出参数的操作柄
    this->libargs_out.args = (vector<block_info> *) dlsym(this->handle, ("__"+name+"_args_out").data());
    if(this->libargs_out.args == nullptr) throw "can not get the HANDLE to GET args";
    
    return 0;
}

CPart::~CPart(){
    Clear();
    if(handle != nullptr) dlclose(handle);
}


/**
 设置计算模块的参数格式便于检查

 @param fargs_in 输入参数格式
 @param fargs_out 输出参数格式
 */
void CPart::setArgsType(vector<farg_info> fargs_in, vector<farg_info> fargs_out){
    this->fargs_in = fargs_in;
    this->fargs_out = fargs_out;
}

/**
 利用传入操作柄向计算模块入口传入参数，并运行计算模块，而后利用传出操作柄从出口获得传出参数

 @return 如果计算过程执行成功则返回SUCCESS
 */
int CPart::Run(void){
    if(func == nullptr) throw "func is nullptr";

//    检查传入参数缓冲区是否正确初始化
    if(args_in.size() != fargs_in.size()) throw "input arg buff is't correctly init";
//    对计算模块传入参数
    for(auto arg : args_in)
        libargs_in.addArgPtr(main_pool.size(arg), arg);
    
//    执行计算过程
    if(func() == SUCCESS){
//        从出口获得传出参数到传出参数缓冲区
        for(auto libarg : *libargs_out.args){
            void *arg = main_pool.bp_malloc(libarg.get_size(), libarg.get_pvle());
            args_out.push_back(arg);
        }
//        清空出口数据
        libargs_out.clear();
        return SUCCESS;
    }
    else return -1;
}


/**
 清空传入参数缓冲区与传出参数缓冲区
 */
void CPart::Clear(void){
//    清空传入参数缓冲区
    for(auto arg : args_in) main_pool.b_free(arg);
    args_in.clear();
//    清空传出参数缓冲区
    for(auto arg : args_out) main_pool.b_free(arg);
    args_out.clear();
}

void CPart::AddCPArgsIn(void *arg){
    void *p_value = main_pool.b_get(arg);
    if(p_value == nullptr) throw "information lost";
    args_in.push_back(p_value);
}

void LibArgsTransfer::addArgPtr(int size, void *p_arg){
    void *pc_arg = malloc(size);
    memcpy(pc_arg, p_arg, size);
    block_info pbifo(size,pc_arg);
    args->push_back(pbifo);
}

void LibArgsTransfer::clear(void){
    for(auto arg : *args)
        free(arg.get_pvle());
    args->clear();
}

LibArgsTransfer::LibArgsTransfer(){
    
}
