//
//  cthread.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cthread_h
#define cthread_h

#include "type.h"
#include "cpart.h"
#include "cmap.h"
#include "server.h"

class CThread;
class Server;

//线程信息记录结构体
struct thread_args{
    //    子线程编号
    unsigned long tid;
    //    指向计算任务
    CThread *pct;
    //    指向计算模块
    CPart *pcp;
    //    储存计算模块调用的返回值
    int rtn;
};

//并行任务处理进程信息结构体
struct line_process{
//  是否开启并行任务管理
    bool if_als = false;
//    已经释放的子线程
    list<struct thread_args *>child_finished;
//    子线程管理状态记录
    map<unsigned long,pthread_t> threads;
//    记录计算模块对应的线程id
    map<const CPart * const,unsigned long> cpttid;
//    计算模块处理队列
    list<CPart *> line;
};

//计算任务类
class CThread{
public:
//    计算任务图纸
    const CMap * const p_map;
//    此计算进程中计算模块的传入参数数据列表
    map<string,vector<void *>> rargs;
//    此计算进程的计算模块的传出参数数据列表
    map<string,vector<void *>> rargs_out;
//    计算模块是否已经解决
    map<string,bool> ifsolved;
//    计算模块是否有数据
    map<string,bool> if_rargs;
//    tid生成的依据
    unsigned long idxtid;
//    并行线程的个数
    int thdnum;
//    并行任务处理进程
    struct line_process lpcs;
//    守护进程定时器
    struct itimerval itrl;
    
    
//    使用图结构管理结构来构造计算进程管理结构
    CThread(CMap *tp_map, int thdnum = 4);
    ~CThread();
    
//    添加相关计算模块的传入参数
    void AddArgs(string name, void *pvle){
        if(main_pool.b_get(pvle) == nullptr) throw "information lost";
        auto argil = rargs.find(name);
        argil->second.push_back(pvle);
    }
    
//    添加相关计算模块的传出参数
    void AddArgsOut(string name, void *pvle){
        if(main_pool.b_get(pvle) == nullptr) throw "information lost";
        auto argol = rargs_out.find(name);
        argol->second.push_back(pvle);
    }
    
//    设置守护进程
    void SetDaemon(void);
    
//    守护进程
    void Daemon(void);
//    分析图结构来构造新的处理队列
    void Analyse(void);
//    执行处理队列
    void DoLine(void);
//    通过子线程所属的模块名找到子线程的id
    long FindChildPCS(string name);
//    取消子线程
    int CancelChildPCS(unsigned long tid);
//    处理数据包
    int GetCPUResult(struct compute_result *);
//    标记计算模块
    static void SignedCpart(void *args, CPart *pcp);
//    导出数据包
    struct compute_result BuildCPUResult(CPart *);
//    发送数据包到服务器
    int SendCPUResult(Server *,struct compute_result);
//    从服务器中获得数据包
    vector<struct compute_result> GetCPURFromServer(Server *);
//    查询计算模块是否已经解决
    CPart *IfCPTSolved(string name);
//    建立新线程执行计算模块
    static void * NewThread(void *);
//    为计算模块的调用准备输入参数
    static void PrepareArgsIn(CThread *pct,CPart *);
//    获得计算模块执行后的输出参数
    static void GetArgsOut(CThread *pct,CPart *);
//    通知计算任务子线程即将结束
    static void ChildThreadFSH(struct thread_args *);
};

//注册任务进程时钟调用
void setTrdClock(CThread *ptd);

#endif /* cthread_h */
