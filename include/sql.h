//
//  sql.h
//  Net
//
//  Created by 胡一兵 on 2019/1/29.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef sql_h
#define sql_h

#include "type.h"

struct SQLTable{
    string name;
    vector<pair<string, string>> colnums;
};
//回调项目封装结构
struct SQLItem{
    vector<string> colnum;
    vector<string> argv;
};
//回调结果封装结构
struct SQLCallBack{
    unsigned long size;
    vector<SQLItem> items;
    string errmsg;
    int sql_rtn;
};

namespace sql {
//    基本回调函数
    int SQLCallBackFunc(void *data, int argc, char **argv, char **azColName);
//    执行SQL命令
    SQLCallBack *sql_exec(sqlite3 *psql, string sql);
//    创建新数据表
    int table_create(sqlite3 *psql, string name, vector<pair<string, string>> colnums);
//    编译插入命令
    int insert_info(sqlite3 *psql, sqlite3_stmt **psqlsmt, string table_name, vector<pair<string, string>>value);
//    生成字符串格式的数据
    string string_type(string str);
//    输出错误信息
    void printError(sqlite3 *psql);
//    执行SQL语句
    int exec(sqlite3 *psql, string sql);
}

#endif /* sql_h */
