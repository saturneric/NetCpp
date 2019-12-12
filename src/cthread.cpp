//
//  cthread.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "memory.h"
#include "cthread.h"

list<CThread *> daemon_list = {};

CThread::CThread(CMap *tp_map,int thdnum):p_map(tp_map),idxtid(0),thdnum(thdnum){
    lpcs.if_als = false;

    for(auto cp : p_map->cparts){
//        构造空的传入与传出参数列表
        vector<void *> args,args_out;
        rargs.insert(pair<string,vector<void *>>(cp.first,args));
        rargs_out.insert(pair<string,vector<void *>>(cp.first,args_out));
//        构造任务进度列表
        ifsolved.insert(pair<string,bool>(cp.first,false));
        if_rargs.insert(pair<string,bool>(cp.first,false));
    }
}

CThread::~CThread(){
    for(auto item : rargs){
        for(auto litem : item.second) main_pool.b_free(litem);
    }
    for(auto item : rargs){
        for(auto litem : item.second)
            if(litem != nullptr) main_pool.b_free(litem);
    }
}

void CThread::Analyse(void){

}

void CThread::DoLine(void){
    for(auto pcp : lpcs.line){
        string name = pcp->name;
        
        vector<void *> args = rargs.find(name)->second;
        vector<farg_info> fargs = pcp->fargs_in;
        vector<farg_info> fargs_out = pcp->fargs_out;
        
        unsigned long ntid = idxtid++;
        pthread_t npdt = 0;
//        创建新线程
        struct thread_args *pt_ta = new struct thread_args({ntid,this,pcp,-1});
        if(pthread_create(&npdt,NULL,&CThread::NewThread,(void *)(pt_ta))){
            throw "fail to create thread";
        }
        lpcs.threads.insert({ntid,npdt});
        lpcs.cpttid.insert({pcp,ntid});
    }

}

void CThread::SetDaemon(void){
    daemon_list.push_back(this);
}

void CThread::Daemon(void){
//    等待线程返回
    for(auto cfh : lpcs.child_finished){
        unsigned long tid = cfh->tid;
        pthread_t cpdt = lpcs.threads.find(tid)->second;
        struct thread_args *rpv = nullptr;
        pthread_join(cpdt, (void **)&rpv);
//        根据返回值处理计算任务状态
        if(rpv->rtn == SUCCESS){
//            标识该计算模块中计算任务的状态为已解决
            ifsolved.find(rpv->pcp->name)->second = true;
//            标识储存有该计算任务的输出参数
            if_rargs.find(rpv->pcp->name)->second = true;
        }
        else{
            
        }
//        释放线程资源
        pthread_detach(cpdt);
//        在列表中销户证实宣告线程程结束
        lpcs.threads.erase(tid);
        lpcs.cpttid.erase(rpv->pcp);
        printf("TID: %lu Deleted.\n",tid);
//        删除线程传入参数
        delete rpv;
    }
    lpcs.child_finished.clear();
    if(lpcs.threads.size() > 0){
        setTrdClock(this);
    }
}

//子线程即将结束时调用
void CThread::ChildThreadFSH(struct thread_args *pta){
    CThread *pct = pta->pct;
    (pct->lpcs).child_finished.push_back(pta);
    printf("Called TID %lu.\n",pta->tid);
}

void *CThread::NewThread(void *pv){
//    取消信号对于线程起作用
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
//    线程收到取消信号时立即取消
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    struct thread_args *pta = (struct thread_args *)pv;
    printf("Calling TID %lu.\n",pta->tid);
//    准备输入参数
    PrepareArgsIn(pta->pct,pta->pcp);
    if(pta->pcp->Run() == SUCCESS){
//        检查线程是否已经被取消
        pthread_testcancel();

//        取消信号对于线程不再起作用，以防参数混乱
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
//        处理输出参数
        GetArgsOut(pta->pct,pta->pcp);
        pta->rtn = SUCCESS;
    }
    else{
        pta->rtn = FAIL;
    }
    CThread::ChildThreadFSH(pta);
    pthread_exit(pv);
}

long CThread::FindChildPCS(string name){
    return lpcs.cpttid.find(p_map->cparts.find(name)->second)->second;
}

void CThread::PrepareArgsIn(CThread *pct,CPart *pcp){
//    读入实际计算进程参数列表中的输入参数
    vector<void *> args = pct->rargs.find(pcp->name)->second;
//    获得输入参数格式
    vector<farg_info> fargs = pcp->fargs_in;
//    清空历史数据
    pcp->Clear();
//    传入输入参数
    for(auto arg : args){
        if(main_pool.b_get(arg) == nullptr) throw "information lost";
        pcp->args_in.push_back(arg);
    }
}


void CThread::GetArgsOut(CThread *pct,CPart *pcp){
//    获得输出参数格式
    vector<farg_info> fargs_out = pcp->fargs_out;
//    获得计算模块中的输出参数列表
    vector<void *> &argso = pcp->args_out;
//    获得实际计算进程输出参数储存列表
    vector<void *> &args_out = pct->rargs_out.find(pcp->name)->second;
    
//    处理输出
    for(auto argo : argso) args_out.push_back(argo);
    
}

int CThread::CancelChildPCS(unsigned long tid){
//    找到子线程的操作柄
    pthread_t pht = lpcs.threads.find(tid)->second;
    pthread_cancel(pht);
//    对线程进行销户操作
    lpcs.threads.erase(tid);
    return 0;
}

int CThread::GetCPUResult(struct compute_result *pcrt){
    ifsolved.find(pcrt->name)->second = true;
//    处理输出参数
    CPart *pcp = p_map->cparts.find(pcrt->name)->second;
    vector<farg_info> farg_out = pcp->fargs_out;
    for(auto argo : *pcrt->args_out) AddArgsOut(pcrt->name, argo);
    
//    处理输入参数
    vector<farg_info> farg_in = pcp->fargs_in;
    for(auto argi : *pcrt->args_in) AddArgs(pcrt->name, argi);
    ifsolved.find(pcrt->name)->second = true;
    if_rargs.find(pcrt->name)->second = true;
    
//    处理关联计算模块
    p_map->CMap::MapThrough(pcp, CThread::SignedCpart,&ifsolved);
    return 0;
}

struct compute_result CThread::BuildCPUResult(CPart *pcp){
    struct compute_result ncpur;
    ncpur.name = pcp->name;
    ncpur.args_in = &rargs.find(ncpur.name)->second;
    ncpur.args_in = &rargs.find(ncpur.name)->second;
    return ncpur;
}


void CThread::SignedCpart(void *args, CPart *pcp){
    map<string,bool> *pifsolved = (map<string,bool> *) args;
    pifsolved->find(pcp->name)->second = true;
}

void setTrdClock(CThread *ptd){
    daemon_list.push_back(ptd);
}
