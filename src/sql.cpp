//
//  sql.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/29.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "sql.h"

namespace sql {
    //    数据库回调函数
    int SQLCallBackFunc(void *data, int argc, char **argv, char **azColName){
        SQLCallBack *pscb = (SQLCallBack *)data;
        if(pscb == NULL){
#ifdef DEBUG
            printf("Illegal Function Calling SQLCallBackFunc\n");
#endif
            throw "illegal calling";
        }
        if(pscb != nullptr){
            pscb->size = argc;
            for(int i = 0; i < argc; i++){
                SQLItem nsi;
                nsi.colnum.push_back(azColName[i]);
                nsi.argv.push_back(argv[i] ? argv[i] : "nil");
                pscb->items.push_back(nsi);
            }
        }
        return 0;
    }
    
    //    SQL语句执行
    SQLCallBack *sql_exec(sqlite3 *psql, string sql){
        SQLCallBack *pscb = new SQLCallBack;
        if(pscb == NULL){
#ifdef DEBUG
            printf("Error In Creating Struct SQLCallBack\n");
#endif
        }
#ifdef DEBUG
        printf("[*]SQL Request %s\n[*]SQL %p\n",sql.data(),psql);
#endif
        char *errmsg;
        pscb->sql_rtn = sqlite3_exec(psql, sql.data(), SQLCallBackFunc, (void *)pscb, &errmsg);
        if(errmsg != NULL) pscb->errmsg = errmsg;
        return pscb;
    }
    
    
    /**
     创建数据库表
     
     @param psql 数据库操作柄
     @param name 数据库表名
     @param colnums 数据库表描述
     @return 执行成功返回0
     */
    int table_create(sqlite3 *psql, string name, vector<pair<string, string>> colnums){
#ifdef DEBUG
        printf("Create SQL Table %s\n",name.data());
#endif
        //        SQL语句
        string sql_quote = "CREATE TABLE "+name;
        sql_quote += "(";
        int idx = 0;
        for(auto colnum : colnums){
            if(idx++) sql_quote += ",";
            sql_quote += colnum.first + " " + colnum.second;
        }
        sql_quote += ");";
        //        执行SQL语句
        sql::exec(psql, "BEGIN;");
        SQLCallBack *pscb = sql_exec(psql, sql_quote);
        sql::exec(psql, "COMMIT;");
        if(pscb->sql_rtn != SQLITE_OK){
#ifdef DEBUG
            printf("[Error]Fail To Create Table %s\n",name.data());
#endif
            throw "fail to create table";
        }
#ifdef DEBUG
        printf("Succeed In Creating Table %s\n",name.data());
#endif
        delete pscb;
        return 0;
    }
    
    int insert_info(sqlite3 *psql, sqlite3_stmt **psqlsmt, string table_name,vector<pair<string, string> >items){
        
        string sql_quote = "INSERT INTO "+table_name;
        //        处理表项名
        sql_quote +="(";
        int idx = 0;
        for(auto item : items){
            if(idx++) sql_quote +=",";
            sql_quote+= item.first;
        }
        sql_quote +=") VALUES(";
        //        处理数据
        idx = 0;
        for(auto item : items){
            if(idx++) sql_quote +=",";
            sql_quote+= item.second;
        }
        sql_quote += ");";
#ifdef DEBUG
        printf("SQL Insert Statment %s\n",sql_quote.data());
#endif
        const char *pzTail;
        int rtn;
        rtn = sqlite3_prepare(psql, sql_quote.data(), -1, psqlsmt, &pzTail);
        if(rtn == SQLITE_OK){
#ifdef DEBUG
            printf("[*]Succeed In Compiling SQL Statment\n");
#endif
            return 0;
        }
        else{
#ifdef DEBUG
            printf("[*]Fail to Compile SQL Statment\n");
#endif
            return -1;
        }
        
    }
    
    string string_type(string str){
        return "\'"+str+"\'";
    }
    
    void printError(sqlite3 *psql){
        if(psql != nullptr){
            const char *error = sqlite3_errmsg(psql);
            int errorcode =  sqlite3_extended_errcode(psql);
            printf("\033[31mSQL Error: [%d]%s\n\033[0m",errorcode,error);
        }
    }
    
    int exec(sqlite3 *psql, string sql){
        return sqlite3_exec(psql, sql.data(), NULL, NULL, NULL);
    }
    
}
