//
//  cproj.h
//  Net
//
//  Created by 胡一兵 on 2019/1/22.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cproj_h
#define cproj_h

#include "type.h"
#include "cpart.h"
#include "sha256_cpp_binding.h"
#include "sql.h"

class Proj;

//检查数据库表专用信息储存结构
struct check_table_column{
    string name;
    string type;
    int notnull;
    int pk;
};

struct setting_file_register{
    vector<string> block_keys;
};

struct stn_register{
    vector<string> stn_keys;
};

struct setting_file_read{
    string key;
    string name;
    string sentence;
    bool if_blk;
    vector<setting_file_read *> childs;
};

struct stn_read{
    string key;
    string value;
};

//配置文件通用方法类
class setting_file{
public:
    //    检查名字是否合法
    static bool if_name_illegal(string str){
        for(auto c:str){
            if(!if_illegal(c)) return false;
        }
        return true;
    }
protected:
//    检查路径或文件
    void check_paths(string main_path, vector<string> paths){
        for(auto path : paths){
            if(!~access((main_path+path).data(),F_OK)) throw path+" is abnormal";
        }
    }
//    检查字符是否合法
    static bool if_illegal(char c){
        if(isalnum(c) || c == '_') return true;
        else return false;
    }
//    寻找保留字
    bool search_key(ifstream &ifsfile,string key){
        string line;
        do{
            ifsfile>>line;
        }while(line.find(key) == string::npos && ifsfile.eof() == false);
        
        if(ifsfile.eof() == true) return false;
        return true;
    }
//    判断参数是否为字符串
    bool if_string(string &arg){
        if(arg[0] == '\"' && arg[arg.size()-1] == '\"') return true;
        else return false;
    }
//    判断下一条命令的有无
    bool if_continue(string &arg){
        if(arg.find(",") != string::npos && arg[arg.size()-1] == ',') return true;
        else return false;
    }
//    消去字符串的双引号
    string cut_string(string &arg){
        return arg.substr(1,arg.size()-2);
    }
    
    int read_file(string path, Byte *buff, uint64_t size){
        ifstream ifsf(path.data(),std::ios::binary);
        char tmp[512] = {0}, *idx = buff;
        uint64_t idx_count = 0;
        while (!ifsf.eof() && idx_count < size) {
            if(size < 512){
                memset(tmp, 0, size);
                ifsf.read(tmp, size);
                memcpy(idx, tmp, size);
                idx_count += size;
            }
            else if (size > 512){
                if(size - idx_count >= 512){
                    memset(tmp, 0, 512);
                    ifsf.read(tmp, 512);
                    memcpy(idx, tmp, 512);
                    idx_count += 512;
                    idx += 512;
                }
                else{
                    memset(tmp, 0, size-idx_count);
                    ifsf.read(tmp, size-idx_count);
                    memcpy(idx, tmp, size-idx_count);
                    idx_count += size-idx_count;
                    idx += size-idx_count;
                }
            }
        }
        ifsf.close();
        return 0;
    }
    
//    消去配置文件中的所有的空字符
    void read_settings(string path, string &tstr){
        struct stat tstat;
        stat(path.data(), &tstat);
        Byte *fbs = (Byte *)malloc(tstat.st_size+5);
        read_file(path, fbs,tstat.st_size);
        for(off_t i = 0; i < tstat.st_size; i++){
            if(isgraph(fbs[i])) tstr += fbs[i];
        }
        free(fbs);
    }
//    读取关键字及代码块
    void read_blocks(string str, setting_file_register &tsfr, vector<setting_file_read *> *blocks){
        string tstr = str;
        string::size_type curs_idx = 0;
        while (tstr.size()) {
//            寻找语句或代码块
            string::size_type sem_idx = tstr.find(";",curs_idx);
//            如果没找到分号则读完
            if(sem_idx == string::npos) break;
            string tmpstr = tstr.substr(curs_idx,sem_idx-curs_idx);
            string::size_type blq_idx = tmpstr.find("{",curs_idx),brq_idx = string::npos;
            bool if_blk = true;
            string pcsstr;
//            如果是语句
            if(blq_idx == string::npos){
                brq_idx = tstr.find(";",curs_idx);
                pcsstr = tstr.substr(curs_idx,brq_idx);
                if_blk = false;
            }
            else{
                int blk_stack = 1;
                for(auto c : tstr){
                    pcsstr.push_back(c);
                    if(c == '{') blk_stack++;
                    else if(c == '}'){
                        blk_stack--;
                        if(blk_stack == 1) break;
                    }
                }
                brq_idx = pcsstr.rfind('}') + 1;
            }
            
            setting_file_read *ptsfbr = new setting_file_read();
//            记录是语句还是信息块
            ptsfbr->if_blk = if_blk;
//            如果是信息块
            if(if_blk){
                string head = pcsstr.substr(0,blq_idx);
                string keystr, namestr;
                string::size_type key_idx;
                
//                检查关键字
                for(auto key:tsfr.block_keys){
                    if((key_idx = head.find(key)) != string::npos){
                        keystr = pcsstr.substr(0,key.size());
                        namestr = pcsstr.substr(key.size(),blq_idx-key.size());
                        if(!if_name_illegal(namestr)){
                            throw "block name is illegal";
                        }
                        ptsfbr->key = keystr;
                        ptsfbr->name = namestr;
                        break;
                    }
                }
                if(ptsfbr->key.empty()) throw "unknown block key";
                blocks->push_back(ptsfbr);
                string inblkstr = pcsstr.substr(blq_idx+1,brq_idx-blq_idx-2);
                read_blocks(inblkstr, tsfr, &blocks->back()->childs);
            }
            else{
//                记录语句
                ptsfbr->sentence = pcsstr;
                blocks->push_back(ptsfbr);
            }

            curs_idx = brq_idx+1;
            tstr = tstr.substr(curs_idx,tstr.size()-curs_idx);
            curs_idx = 0;
        }
    }
    int read_stn(string stn_str, stn_register &tsr,stn_read *stn){
        string::size_type key_idx = string::npos;
        stn->key.clear();
        stn->value.clear();
        for(auto key:tsr.stn_keys){
            if((key_idx = stn_str.find(key)) != string::npos){
                stn->key = key;
                stn->value = stn_str.substr(key.size(),stn_str.size()-key.size());
                break;
            }
        }
        if(stn->key.empty()) return -1;
        return 0;
    }
};


struct cpt_func_args{
    string type;
    int size;
    string key;
};

//cpt文件管理类
class Cpt:public setting_file{
    friend Proj;
//    Cpt文件中所提到的源文件
    vector<string> src_files;
//    入口函数对应的源文件（入口函数名，源文件名）
    map<string,string> funcs_src;
//    入口函数的输入与输出参数格式(入口函数名，参数列表)
    map<string,vector<cpt_func_args>> fargs_in,fargs_out;
//    Cpt文件路径
    string path;
    ifstream ifscpt;
//    工程名
    string name;
//    处理参数列表
    vector<cpt_func_args> deal_args(string args);
//    处理参数
    cpt_func_args deal_arg(string arg);
//    配置文件文件解析结构
    vector<setting_file_read *> blocks;
//    文件数据
    string content;
//    处理文件数据
    void deal_content(string data_content);
public:
//    构造函数
    Cpt(string path, string proj_name);
//    数据库数据直接构造函数
    Cpt(string data_content, int if_db, string proj_name);
};

//map文件管理类
class Map: public Cpt{
    
};


//proj文件管理类
class Proj:public setting_file{
//    计算工程所在的目录
    string proj_path;
//    计算工程描述文件名
    string proj_file;
//    工程名
    string name;
//    计算工程读入流
    ifstream ifsproj;
//    工程文件内容
    string content;
//    源文件所在的目录
    vector<string> src_paths;
//    源文件搜索目录下的所有源文件
    map<string,int> src_files;
//    计算工程所涉及到的源文件
    map<string,int> used_srcfiles;
//    计算工程所涉及到的源文件的MD5
    map<string,string> src_md5;
//    关系描述文件所在的目录
    vector<string> map_paths;
//    模块描述文件所在目录
    vector<string> cpt_paths;
//    模块描述对象
    vector<Cpt *> cpts;
//    关系描述对象
    vector<Map> maps;
//    动态链接库存放的目录
    string lib_path;
//    模块入口函数索引
    map<string, Cpt *> func_index;
//    动态链接库对应的源文件索引
    map<string, string> lib_index;
//    工程对应数据库
    sqlite3 *psql;
//    数据库文件路径
    string db_path;
//    配置文件文件解析结构
    vector<setting_file_read *> blocks;

//    处理描述文件的命令
    void deal_order(string tag, string arg);
//    读取所有涉及的Cpt文件
    void build_cpts(void);
//    检查Cpt文件中描述的源文件是否存在对应实体
    void check_cpt(void);
//    搜寻源文件目录
    void search_src(int idx,string path);
//    将现有信息储存到一个新的数据库中
    void update_db(void);
//    检查涉及到的源文件的MD5与数据库中的是否一致
    void check_src_md5(string db_path);
//    编译目标源文件生成动态链接库
    void build_src(string lib_name,string srcfile_path, string libs_path);
//    创建数据库
    void build_new_db(void);
//    写入项目涉及的源文件信息到数据库中
    void write_src_info(void);
//    写入动态链接库信息到数据库中
    void write_lib_info(void);
//    写入入口函数信息到数据库中
    void write_func_info(void);
    void write_func_info(string func_name, Cpt *pcpt);
//    写入口函数入输入输出参数信息到数据库中
    void write_args_info(void);
    void write_args_info(string func_name, Cpt *pcpt);
    void write_args_info_no_create_table(string func_name, Cpt *pcpt);
//    写入cpt文件信息到数据库中
    void write_cpt_info(void);
//    写入工程描述文件信息到数据库中
    void write_proj_info(void);
//    检查数据库是否正确
    void check_database(void);
//    检查数据库表
    void check_table(int cnum, vector<check_table_column> tctc,sqlite3_stmt *psqlsmt);
//    解析数据
    void deal_content(string data_content);
//    编译源文件
    void compile_srcfile(string src_name, string src_path);
public:
//    读取Proj文件
    Proj(string t_projpath, string t_projfile);
//    接受数据库数据
    Proj(string data_content);
//    检查目录以及描述文件是否存在
    void GeneralCheckInfo(void);
//   搜寻源文件搜索目录并读取Cpt文件
    void SearchInfo(void);
//    检查Cpt文件内所描述的源文件是否有对应的实体
    void CheckCptInfo(void);
//    建立计算模块入口函数索引
    void BuildFuncIndex(void);
//    编译涉及到的源文件
    void CompileUsedSrcFiles(void);
//    检查入口函数是否在对应的动态链接库中可被按要求正确解析
    void CheckFuncInfo(void);
//    建立数据库
    void DBProcess(void);
//    获得工程名
    string GetName(void);
//    更新工程
    void UpdateProcess(void);
//    获得数据库
    void AttachDatabases(void);
};

#endif /* cproj_h */
