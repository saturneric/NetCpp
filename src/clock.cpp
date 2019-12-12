//
//  clock.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "clock.h"

#define CLOCKESE 30

list<clock_register *> clocks_list,reset_clocks;
map<uint32_t,clock_thread_info *> clocks_thread_map;

list<uint32_t> clock_thread_finished;
static struct itimerval oitrl, itrl;
uint32_t tid_r = 0;

static uint64_t clock_erase = CLOCKESE;


void initClock(void){
    signal(SIGALRM, threadsClock);
    setThreadsClock();
}

//设置全局线程时钟
void setThreadsClock(void){
    itrl.it_interval.tv_sec = 0;
    itrl.it_interval.tv_usec = 50000;
    itrl.it_value.tv_sec = 0;
    itrl.it_value.tv_usec = 50000;
    setitimer(ITIMER_REAL, &itrl, &oitrl);
}

void newClock(clock_register *pncr){
    clocks_list.push_back(pncr);
}

//时钟滴答调用函数
void threadsClock(int n){
//    处理已完成线程
    for(auto tid : clock_thread_finished){
        clock_thread_info *tcti = clocks_thread_map.find(tid)->second;
        pthread_join(tcti->pht,NULL);
        pthread_detach(tcti->pht);
        clocks_thread_map.erase(clocks_thread_map.find(tid));
        delete tcti;
    }
    clock_thread_finished.clear();
    
//    删除到期时钟
    if(clock_erase == 0){
        //printf("Cleaning clocks.\n");
        clocks_list.remove_if([](clock_register *pclock){return pclock == NULL;});
//        重设总滴答数
        clock_erase = CLOCKESE;
    }
    else clock_erase--;
    
//    处理时钟列表
    for(auto &pclock : clocks_list){
        if(pclock == NULL) continue;
        if(pclock->click == 0){
            clock_thread_info *pncti = new clock_thread_info();
            pncti->args = pclock->arg;
            pncti->tid = tid_r++;
            pclock->if_thread = 1;
            clocks_thread_map.insert({pncti->tid,pncti});
            
            pthread_create(&pncti->pht, NULL, pclock->func, pncti);

//            标记时钟到期
            if(pclock->if_reset){
                pclock->click = pclock->rawclick;
            }
            else{
                delete pclock;
                pclock = NULL;
            }
            
        }
        else if(pclock->click > 0){
            pclock->click--;
        }
    }
    
}

void clockThreadFinish(uint32_t tid){
    //    屏蔽时钟信号
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs,SIGALRM);
    sigprocmask(SIG_BLOCK,&sigs,0);
    clock_thread_finished.push_back(tid);
    sigprocmask(SIG_UNBLOCK,&sigs,0);
}

