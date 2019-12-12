//
//  instruct.h
//  Net
//
//  Created by 胡一兵 on 2019/2/5.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef instruct_h
#define instruct_h

#include "type.h"
#include "memory.h"
#include "clock.h"
#include "net.h"
#include "cproj.h"
#include "cpart.h"
#include "cmap.h"
#include "cthread.h"
#include "sha1.h"
#include "rsa.h"
#include "rng.hpp"

#include <boost/program_options.hpp>

struct instructions{
    int (*unpack)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*construct)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*update)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*server)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*client)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*set)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*init)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
};

int update(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
int construct(string instruct,vector<string> &config, vector<string> &lconfig, vector<string> &target);
int server(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
int client(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
int init(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
int set(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);


bool config_search(vector<string> &configs,string tfg);
void getSQEPublicKey(respond *pres,void *args);
void registerSQECallback(respond *pres,void *args);
void loginSQECallback(respond *pres, void *args);
void* connectionDeamon(void *args);
//客户端连接管理守护进程
void *clientServiceDeamon(void *);
//实用函数
void gets_s(char *buff, uint32_t size);

#endif /* instruct_h */
