//
//  CProj.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/22.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cproj.h"

Cpt::Cpt(string path, string proj_name){
#ifdef DEBUG
    printf("Reading Cpt File %s\n[*]Require Project Name %s\n",path.data(),proj_name.data());
#endif
    string tifscpt;
    read_settings(path, tifscpt);
    name = proj_name;
    deal_content(tifscpt);
}

Cpt::Cpt(string data_content, int if_db, string proj_name){
#ifdef DEBUG
    printf("Receive Data To Build Class Cpt\n [*]%s\n",data_content.data());
#endif
    name = proj_name;
    deal_content(data_content);
}

void Cpt::deal_content(string data_content){
    setting_file_register tsfr = {
        {
            "cparts",
            "srcfile",
        }
    };
    //    解析数据
    read_blocks(data_content, tsfr, &blocks);
    content = data_content;
    bool if_pblk = false;
    int idx = 0;
    //    寻找工程对应的块
    for(auto block :blocks){
        if(block->name == name){
            if_pblk = true;
            break;
        }
        idx++;
    }
    if(!if_pblk) throw "proper blocks not found";
    if(blocks[idx]->if_blk){
        for(auto src : blocks[idx]->childs){
            if(src->key == "srcfile"){
//                记录源文件名
                string tsrc_name = src->name+".cpp";
                src_files.push_back(tsrc_name);
                for(auto func : src->childs){
                    if(!func->if_blk){
//                        分离输出参数列表
                        string::size_type dq_l = func->sentence.find("(");
                        string::size_type dq_r = func->sentence.find(")");
                        if(dq_l == string::npos || dq_r == string::npos) throw "syntax error";
                        string str_argout = func->sentence.substr(dq_l+1,dq_r-dq_l-1);
#ifdef DEBUG
                        printf("Read Args (OUT) Description %s\n",str_argout.data());
#endif
                        vector<cpt_func_args> cfgo = deal_args(str_argout);
//                        分离输入参数列表
                        string::size_type yq_l = func->sentence.find("(",dq_r+1);
                        string::size_type yq_r = func->sentence.find(")",yq_l);
                        if(yq_l == string::npos || yq_r == string::npos) throw "syntax error";
                        string str_argin = func->sentence.substr(yq_l+1,yq_r-yq_l-1);
#ifdef DEBUG
                        printf("Read Args (IN) Description %s\n",str_argin.data());
#endif
                        vector<cpt_func_args> cfgi = deal_args(str_argin);
                        
//                        分离入口函数名
                        string func_name = func->sentence.substr(dq_r+1,yq_l-dq_r-1);
                        if(!if_name_illegal(func_name)) throw "function name is illegal";
                        
#ifdef DEBUG
                        printf("Read Function Name %s\n",func_name.data());
#endif
//                        记录入口函数名
                        funcs_src.insert({func_name,tsrc_name});
//                        添加相关参数
                        fargs_out.insert({func_name,cfgo});
                        fargs_in.insert({func_name,cfgi});
                        
                    }
                }
            }
        }
    }
}


vector<cpt_func_args> Cpt::deal_args(string args){
    string::size_type lcma_dix = 0;
    string::size_type cma_idx = args.find(",",0);
    vector<cpt_func_args> cfgs;
    
    if(cma_idx == string::npos){
        string real_args;
        for(auto c : args){
            if(isgraph(c)){
                real_args.push_back(c);
                if(!if_illegal(c)) throw "func name has illegal char";
            }
        }
        if(real_args.size() > 3){
            cpt_func_args ncfg = deal_arg(args);
            cfgs.push_back(ncfg);
        }
    }
    else{
        //            分割逗号
        while(cma_idx != string::npos){
            string arg = args.substr(lcma_dix,cma_idx-lcma_dix);
            cpt_func_args ncfg;
            ncfg = deal_arg(arg);
            cfgs.push_back(ncfg);
            lcma_dix = cma_idx+1;
            cma_idx = args.find(",",lcma_dix);
            if(cma_idx == string::npos && lcma_dix != string::npos){
                arg = args.substr(lcma_dix,args.size()-lcma_dix);
                ncfg = deal_arg(arg);
                cfgs.push_back(ncfg);
            }
        }
    }
    return cfgs;
}

cpt_func_args Cpt::deal_arg(string arg){
    cpt_func_args ncfa;
    std::stringstream ss,sr;
    stn_register tsr = {
        {
            "int",
            "double"
        }
    };
//        读取数组标号
    string::size_type fq_l = arg.find("[");
    string::size_type fq_r = arg.find("]");
    int size = 1;
    string type;
    if(fq_l != string::npos && fq_r != string::npos){
        string size_str = arg.substr(fq_l+1,fq_r-fq_l-1);
        sr<<size_str;
        sr>>size;
        type = arg.substr(0,fq_l);
        ncfa.size = size;
        ncfa.type = type;
        string key = arg.substr(fq_r+1,arg.size()-fq_r-1);
        ncfa.key = key;
    }
    else if (fq_r == string::npos && fq_r == string::npos){
        stn_read nstnr;
        read_stn(arg, tsr, &nstnr);
        ncfa.type = nstnr.key;
        ncfa.key = nstnr.value;
        ncfa.size = 1;
    }
    else throw "syntax error";
    
    return ncfa;
}
