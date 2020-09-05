//
//  cmap.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cmap_h
#define cmap_h

#include "type.h"
#include "cpart.h"
#include "util/sha256_generator.h"
#include "sql.h"

//计算模块管理对象间的依赖关系管理结构
class cp_depend{
public:
//    指向某计算模块对象
    CPart *f_cpart;
//    记录所依赖的子计算模块对象及其参数信息
    map<CPart *, vector<int> > cdpd;
//    记录其父计算模块对象及其参数信息
    map<CPart *, vector<int> > fdpd;
};




//计算任务图类
class CMap{
public:
//    计算任务图中包含的的计算模块列表
    map<string,CPart *> cparts;
//    记录计算模块依赖关系图
    map<CPart *, cp_depend> depends;
//    构造函数传入计算工程管理类
    CMap(class Proj &);
//    根据图的表述文件构造计算模块列表
    void BuildCPart(ifstream &map);
//    根据图表述文件中的描述信息，处理并转化为形式输入或输出参数列表
    vector<int> BuidArgs(string &line);
//    根据图的表述文件构造计算模块之间的依赖关系
    void BuildConnection(ifstream &map);
//    根据图描述文件依赖关系描述语句所提供的信息转化为依赖关系结构
//    Depends ReadItem(string item);
    
//    由某个节点递归向下遍历
    static void MapThrough(CPart *pcp,void(*func)(void *,CPart *),void *);
    
};

#endif /* cmap_h */
