//
//  controller.cpp
//  Net
//
//  Created by 胡一兵 on 2019/2/8.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "instruct.h"

extern string PRIME_SOURCE_FILE;



//线程阻塞开关
int if_wait = 1;

//工具组初始化
int init(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;

	//连接数据库
    sqlite3_open("info.db", &psql);
    const char *pzTail;
	//对于服务器的初始化
    if(targets[0] == "server"){
		if (targets.size() < 3) {
			error::printError("Illegal Args.\nFromat: init server [server_name] [key]");
			return -1;
		}

		//检查名字是否合乎规范
		if (!setting_file::if_name_illegal(targets[0].data())) {
			error::printError("Illegal Arg server_name.");
			return -1;
		}

		try {
			//创建数据库服务器信息描述数据表
			sql::table_create(psql, "server_info", {
				{"name","TEXT"},
				{"sqes_public","NONE"},
				{"sqes_private","NONE"},
				{"key_sha1","TEXT"}
				});
		}
		catch (const char * errinfo) {
			string errstr = errinfo;
			if (errstr == "fail to create table") {
				if (!config_search(configs, "-f")) {
					error::printWarning("Have already init server information.\nUse arg \"-f\" to continue.");
					return 0;
				}
				else{
					string sql_quote = "DELETE FROM server_info;";
					sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
					int rtn = sqlite3_step(psqlsmt);
					if (rtn == SQLITE_DONE) {

					}
					else {
						const char *error = sqlite3_errmsg(psql);
						int errorcode = sqlite3_extended_errcode(psql);
						printf("\033[31mSQL Error: [%d]%s\n\033[0m", errorcode, error);
						throw error;
					}
					sqlite3_finalize(psqlsmt);
				}
			}
		}

		//构建数据库插入命令
		sql::insert_info(psql, &psqlsmt, "server_info", {
				{"sqes_public","?1"},
				{"sqes_private","?2"},
				{"key_sha1","?3"},
				{"name","?4"},
		});

        struct public_key_class npbkc;
        struct private_key_class nprkc;

		//生成RSA钥匙串
        rsa_gen_keys(&npbkc, &nprkc, PRIME_SOURCE_FILE);
		
		//填写数据库数据表
        sqlite3_bind_blob(psqlsmt, 1, &npbkc, sizeof(public_key_class), SQLITE_TRANSIENT);
        sqlite3_bind_blob(psqlsmt, 2, &nprkc, sizeof(private_key_class), SQLITE_TRANSIENT);
		sqlite3_bind_blob(psqlsmt, 4, targets[1].data(), targets[1].size(), SQLITE_TRANSIENT);
		
		//生成服务器访问口令哈希码(SHA1)
        if(targets[2].size() < 6) error::printWarning("Key is too weak.");
        string sha1_hex;
        SHA1_Easy(sha1_hex, targets[1]);
        sqlite3_bind_text(psqlsmt, 3, sha1_hex.data(), -1, SQLITE_TRANSIENT);
        
		//执行数据库写入命令
        if(sqlite3_step(psqlsmt) != SQLITE_DONE){
            sql::printError(psql);
        }
        sqlite3_finalize(psqlsmt);
		

		//输出成功信息
        error::printSuccess("Succeed.");
		
        sqlite3_close(psql);
        return 0;
        
    }
    else{
		//对于客户端的初始化
		if(targets.size() < 2) {
			error::printError("Illegal Args.\nFromat: init [client_name] [client_tag]");
			return -1;
		}

		//检测名字与标签是否符合规范
		if (setting_file::if_name_illegal(targets[0]));
		else {
			error::printError("Illegal Arg client_name.");
			return -1;
		}
		if (setting_file::if_name_illegal(targets[1]));
		else {
			error::printError("Illegal Arg client_tag.");
			return -1;
		}

        try {
			//创建客户端描述信息数据表
            sql::table_create(psql, "client_info", {
                {"name","TEXT"},
                {"tag","TEXT"},
                {"admin_key_sha1","TEXT"},
                {"msqes_ip","TEXT"},
                {"msqes_port","INT"},
                {"msqes_key","TEXT"},
                {"msqes_rsa_public","NONE"},
            });
            sql::table_create(psql, "sqes_info", {
                {"sqes_ip","TEXT PRIMARY KEY"},
                {"sqes_port","INT"},
                {"sqes_key","TEXT"},
                {"rsa_public","NONE"},
            });
        } catch (const char *error_info) {
            if(!strcmp(error_info, "fail to create table")){
				//检测强制参数
                if(!config_search(configs, "-f")){
                    printf("\033[33mWarning: Have Already run init process.Try configure -f to continue.\n\033[0m");
                    return 0;
                }
                else{
					//	清空已存在的数据表
                    string sql_quote = "DELETE FROM client_info;";
                    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
                    int rtn = sqlite3_step(psqlsmt);
                    if(rtn == SQLITE_DONE){
                        
                    }
                    else{
                        const char *error = sqlite3_errmsg(psql);
                        int errorcode =  sqlite3_extended_errcode(psql);
                        printf("\033[31mSQL Error: [%d]%s\n\033[0m",errorcode,error);
                        throw error;
                    }
                    sqlite3_finalize(psqlsmt);
                }
            }
        }
        
    }
    
    //构建数据库插入命令
    sql::insert_info(psql, &psqlsmt, "client_info", {
        {"name","?1"},
        {"tag","?2"}
    });
    sqlite3_bind_text(psqlsmt, 1, targets[0].data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(psqlsmt, 2, targets[1].data(), -1, SQLITE_TRANSIENT);
    int rtn = sqlite3_step(psqlsmt);
    if(rtn == SQLITE_DONE){
        
    }
    else throw "sql writes error";
    sqlite3_finalize(psqlsmt);
    sqlite3_close(psql);

	//成功执行
	error::printSuccess("Succeed.");
    return 0;
}

//修改工具组配置信息
int set(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    if(targets.size() < 2){
        error::printError("Illegal Args.\nUse help to get more information.");
        return -1;
    }

    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;
    const char *pzTail;

	//连接数据库
    if(sqlite3_open("info.db", &psql) == SQLITE_ERROR){
        sql::printError(psql);
        return -1;
    }
    string sql_quote = "SELECT count(*) FROM sqlite_master WHERE name = 'client_info';";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    int if_find = sqlite3_column_int(psqlsmt, 0);
    if(if_find);
    else{
        error::printError("Couldn't SET before INIT.");
        return -1;
    }
    sqlite3_finalize(psqlsmt);

    if(targets[0] == "server"){
		if (targets.size() < 3) {
			error::printError("Illegal Args.\nFromat set server [server_ip] [server_port].");
			return -1;
		}
        sql_quote = "UPDATE client_info SET msqes_ip = ?1, msqes_port = ?2 WHERE rowid = 1;";
        sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
        //检查广场服务器IP地址是否正确
        if(!Addr::checkValidIP(targets[1])){
            error::printError("Arg(ipaddr) is abnomal.");
            sqlite3_finalize(psqlsmt);
            sqlite3_close(psql);
            return -1;
        }
        sqlite3_bind_text(psqlsmt, 1, targets[1].data(), -1, SQLITE_TRANSIENT);
        //获得广场服务器端口
        stringstream ss;
        ss<<targets[2];
        int port;
        ss>>port;
        if(port > 0 && port <= 65535);
        else{
            error::printError("Arg(port) is abnomal.");
            sqlite3_finalize(psqlsmt);
            sqlite3_close(psql);
            return -1;
        }
        sqlite3_bind_int(psqlsmt, 2, port);

		//执行数据库指令
        int rtn = sqlite3_step(psqlsmt);
        if(rtn != SQLITE_DONE){
            sql::printError(psql);
        }
        sqlite3_finalize(psqlsmt);
    }
    else if (targets[0] == "key"){
		if (targets.size() < 3) {
			error::printError("Illegal Args.\nFromat set key [key_type] [key]");
			return -1;
		}
		//客户端远程管理口令
        if(targets[1] == "admin"){
            string hexresult;
            SHA1_Easy(hexresult, targets[2]);
            if(targets[1].size() < 6){
                error::printWarning("Key is too weak.");
            }
            sql_quote = "UPDATE client_info SET admin_key_sha1 = ?1 WHERE rowid = 1;";
            sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
            sqlite3_bind_text(psqlsmt, 1, hexresult.data(), -1, SQLITE_TRANSIENT);
            if(sqlite3_step(psqlsmt) != SQLITE_DONE){
                sql::printError(psql);
            }
            sqlite3_finalize(psqlsmt);
        }
		//广场服务器访问口令
        else if(targets[1] == "server"){
            sql_quote = "UPDATE client_info SET msqes_key = ?1 WHERE rowid = 1;";
            sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
            sqlite3_bind_text(psqlsmt, 1, targets[2].data(), -1, SQLITE_TRANSIENT);
            if(sqlite3_step(psqlsmt) != SQLITE_DONE){
                sql::printError(psql);
            }
            sqlite3_finalize(psqlsmt);
        }
        else{
            error::printError("Args(type) is abnormal.");
            return -1;
        }
    }
	else {
		error::printError("Operation doesn't make sense.");
		return 0;
	}
    error::printSuccess("Succeed.");
    sqlite3_close(psql);
    return 0;
}

int server(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    initClock();
    setThreadsClock();
	signal(SIGPIPE, SIG_IGN);
    if(targets.size() == 0){
        //Server nsvr;
        //setServerClock(&nsvr, 3);
		SQEServer nsvr;
		setServerClockForSquare(&nsvr, 3);
    }
    else{
        if(targets[0] == "square"){
            SQEServer nsvr;
            setServerClockForSquare(&nsvr, 3);
        }
    }
    while(1) usleep(1000000);
    return 0;
}

int update(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    try {
        Proj nproj(targets[0], "netc.proj");
        nproj.UpdateProcess();
    } catch (const char *err_info) {
        printf("\033[31mError: %s\n\033[0m",err_info);
        return -1;
    }
    printf("\033[32mSucceed.\n\033[0m");
    return 0;
}

int construct(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    try{
        //        读取工程文件
        Proj nproj(targets[0],"netc.proj");
        //        检查数据库文件是否存在
        string tdb_path = targets[0] + "/dbs/" + nproj.GetName() +".db";
#ifdef DEBUG
        printf("Search Database %s\n",tdb_path.data());
#endif
        if(!access(tdb_path.data(), R_OK)){
            //            设置为强制执行
            if(config_search(configs, "-f")){
                if(remove(tdb_path.data()) == -1){
                    printf("\033[31m");
                    printf("Error: Process meet unknown error.\n");
                    printf("\033[0m");
                    return -1;
                }
            }
            else{
                printf("\033[33m");
                printf("Warning:Database has already existed. Use -f to continue process.\n");
                printf("\033[0m");
                return 0;
            }
        }
        //        总体信息检查
        nproj.GeneralCheckInfo();
        //        收集信息
        nproj.SearchInfo();
        //        构建入口函数索引
        nproj.BuildFuncIndex();
        //        检查cpt文件信息
        nproj.CheckCptInfo();
        //        编译涉及源文件
        nproj.CompileUsedSrcFiles();
        //        检查入口函数信息
        nproj.CheckFuncInfo();
        //        建立数据库
        nproj.DBProcess();
    }
    catch(char const *error_info){
        printf("\033[31mError:");
        printf("%s\033[0m\n",error_info);
        return -1;
    }
    printf("\033[32mSucceed.\n\033[0m");
    return 0;
}

int client(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
	
    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;
    const char *pzTail;

    if(sqlite3_open("info.db", &psql) == SQLITE_ERROR){
        sql::printError(psql);
        return -1;
    }
	
//    初始化时钟
    initClock();
    setThreadsClock();
    
//    建立客户端
    Client nclt(9050);
	
    bool if_setip = false;
    string set_ip;
	
    
    if(config_search(configs, "-p")){
        set_ip = targets[0];
        printf("Set IP: %s\n",set_ip.data());
        if_setip = true;
    }
	
    setClientClock(&nclt, 3);
	
    request *preq;
    

//    获得主广场服务器的通信公钥
    string sql_quote = "select count(*) from client_info where rowid = 1 and msqes_rsa_public is null;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    int if_null = sqlite3_column_int(psqlsmt, 0);
    sqlite3_finalize(psqlsmt);

//    获得主广场服务器的ip地址及其通信端口
    string msqe_ip;
    int msqe_port;
    sql_quote = "select msqes_ip,msqes_port from client_info where rowid = 1;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    msqe_ip = (const char *)sqlite3_column_text(psqlsmt, 0);
    msqe_port = sqlite3_column_int(psqlsmt, 1);
    sqlite3_finalize(psqlsmt);
	error::printSuccess("Main Server IP: " + msqe_ip);
	error::printSuccess("Main Server Port: " + std::to_string(msqe_port));
	
//    如果本地没有主广场服务器的公钥
    if(if_null){
		//向广场服务器申请通信公钥
        nclt.NewRequest(&preq, msqe_ip, msqe_port, "public request", "request for public key");
        nclt.NewRequestListener(preq, 30, psql, getSQEPublicKey);
        if_wait = 1;

		//等待广场服务器回应
        while (if_wait == 1) {
            usleep(1000);
        }
        if(!if_wait){
#ifdef DEBUG
            printf("Succeed In Getting Rsa Public Key From SQEServer.\n");
#endif
			error::printSuccess("Succeed In Requesting Public Key.");
        }
        else{
#ifdef DEBUG
            printf("Error In Getting Rsa Public Key From SQEServer.\n");
#endif
			error::printError("Fail To Request Public Key.");
            throw "connection error";
            return -1;
        }
    }
	
//    获得与广场服务器的通信的公钥
    sql_quote = "select msqes_rsa_public from client_info where rowid = 1;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    public_key_class *ppbc = (public_key_class *)sqlite3_column_blob(psqlsmt, 0);
    nclt.SetPublicKey(*ppbc);
    sqlite3_finalize(psqlsmt);
    
	//检测本地的注册信息
	sql_quote = "select count(name) from sqlite_master where name = \"client_register_info\"";
	sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
	if (sqlite3_step(psqlsmt) != SQLITE_ROW) {
		sql::printError(psql);
		throw "database is abnormal";
	}
	int if_find = sqlite3_column_int(psqlsmt, 0);
	if (if_find) {
		error::printInfo("Doing Login");
		//如果本地已经有注册信息
		string reqstr = " {\"passwd\":null, \"client_id\":null}";
		string sql_quote = "SELECT * from client_register_info where rowid = 1;";
		sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
		sqlite3_step(psqlsmt);
		int status = sqlite3_column_int(psqlsmt, 5);
		if (status == 1) {
			error::printSuccess("[GET INFO]");
			uint64_t client_id = sqlite3_column_int64(psqlsmt, 0);
			uint64_t passwd = sqlite3_column_int64(psqlsmt, 4);
			string name = (const char *)sqlite3_column_text(psqlsmt, 1);
			string tag = (const char *)sqlite3_column_text(psqlsmt, 2);

			aes_key256 naeskey;
			const void *key_buff = sqlite3_column_blob(psqlsmt, 3);
			memcpy(naeskey.key, key_buff, sizeof(uint64_t) * 4);
			nclt.SetAESKey(naeskey);

			error::printInfo("Client_ID: " + std::to_string(client_id));
			error::printInfo("Passwd: " + std::to_string(passwd));
			error::printInfo("Name: " + name);
			error::printInfo("Tag: " + tag);
			
			request *pnreq;
			nclt.NewRequest(&pnreq, msqe_ip, msqe_port, "client login", "", true);
			pnreq->JsonParse(reqstr);
			pnreq->req_doc["client_id"].SetInt64(client_id);
			pnreq->req_doc["passwd"].SetInt64(passwd);
			pnreq->Json2Data();

			nclt.NewRequestListener(pnreq, 44, psql, loginSQECallback);
			//等待主广场服务器回应
			if_wait = 1;
			while (if_wait == 1) {
				sleep(1);
			}
			//成功注册
			if (!if_wait) {

			}

		}
		else {
			error::printError("Register information is broken. Strat to do register.");
			if_find = 0;
		}
		sqlite3_finalize(psqlsmt);

	}
	if(!if_find){
		error::printInfo("Doing Register");
		//如果本地没有注册信息
		//向主广场服务器注册
		aes_key256 naeskey;
		nclt.SetAESKey(naeskey);

		string reqstr = " {\"key\":null, \"name\":null, \"tag\":null, \"sqe_key\":null, \"listen_port\": null,\"listen_ip\":null}";

		Document reqdata;
		if (reqdata.Parse(reqstr.data()).HasParseError()) throw "fail to parse into json";

		//    生成并传递端对端加密报文密钥
		reqdata["key"].SetArray();
		Value &tmp_key = reqdata["key"];
		const uint8_t *p_key = naeskey.GetKey();
		Document::AllocatorType& allocator = reqdata.GetAllocator();
		for (int idx = 0; idx < 32; idx++) {
			tmp_key.PushBack(p_key[idx], allocator);
		}


		reqdata["name"].SetString(nclt.name.data(), (uint32_t)nclt.name.size());
		reqdata["tag"].SetString(nclt.tag.data(), (uint32_t)nclt.tag.size());
		reqdata["sqe_key"].SetString(nclt.sqe_key.data(), (uint32_t)nclt.sqe_key.size());
		//设置TCP监听端口
		reqdata["listen_port"].SetInt(9052);


		//如果强制指定客户端IP地址
		string ip;
		if (if_setip) ip = set_ip;
		else ip = "127.0.0.1";

		reqdata["listen_ip"].SetString(ip.data(), (uint32_t)ip.size());


		//构造请求
		StringBuffer strbuff;
		Writer<StringBuffer> writer(strbuff);
		reqdata.Accept(writer);
		string json_str = strbuff.GetString();

		printf("Connecting...\n");
		//    已获得主广场服务器的密钥，进行启动客户端守护进程前的准备工作
		nclt.NewRequest(&preq, msqe_ip, msqe_port, "private request", json_str, true);
		nclt.NewRequestListener(preq, 44, psql, registerSQECallback);

		//等待主广场服务器回应
		if_wait = 1;
		while (if_wait == 1) {
			sleep(1);
		}
		//成功注册
		if (!if_wait) {
			sqlite3_stmt *psqlsmt;
            sql::exec(psql, "BEGIN IMMEDIATE;");
			sql::insert_info(psql, &psqlsmt, "client_register_info", {
				{"name","?1"},
				{"tag","?2"},
				{"key","?3"},
				{"status","0"},
			});
			sqlite3_bind_text(psqlsmt, 1, nclt.name.data(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(psqlsmt, 2, nclt.tag.data(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_blob(psqlsmt, 3, nclt.post_key.GetKey(), sizeof(uint64_t) * 4, SQLITE_TRANSIENT);
			sqlite3_step(psqlsmt);
			sqlite3_finalize(psqlsmt);
            sql::exec(psql, "COMMIT;");
		}
		else if (~!if_wait) {
			error::printError("fail to do register.");
			return -1;
		}
	}
	//得到服务器回应
    if (!if_wait) {
		
//        成功注册或者登录
        printf("Get Respond From Server.\n");
		sqlite3_close(psql);

//        创建守护进程
        int shmid = shmget((key_t)9058, 1024, 0666|IPC_CREAT);
        if(shmid == -1){
            printf("SHMAT Failed.\n");
        }

        pid_t fpid = fork();
        if(fpid == 0){
			//守护进程
            printf("Client Register Deamon Has Been Created.\n");
			sqlite3 *psql;
			sqlite3_open("info.db", &psql);
            nclt.server_cnt = new SocketTCPCServer(9052);
            nclt.server_cnt->Listen();

			//获得共享内存地址
            Byte *buff = (Byte *)shmat(shmid, NULL, 0);
            if(shmid == -1){
                printf("SHMAT Failed.\n");
            }

			//创建客户端连接管理线程
			pthread_t beat_pid = 0, listen_pid = 0, send_pid = 0;
			connection_listener *pncl = new connection_listener();
			pncl->client_addr = nclt.server_cnt->GetClientAddr();
			pncl->data_sfd = nclt.server_cnt->GetDataSFD();
			pncl->key = nclt.post_key;
			pncl->father_buff = buff;
			pncl->server_cnt = nclt.server_cnt;
			pncl->beat_pid = &beat_pid;
			pncl->listen_pid = &listen_pid;
			pncl->send_pid = &send_pid;
			pncl->p_ci = new connection_info();
			pncl->psql = psql;

			pthread_create(&pncl->pid, NULL, clientServiceDeamon, pncl);
			
			memset(buff, 0, sizeof(uint32_t));
            while (1) {
				//获得连接状态
				if (!memcmp(buff, "CIFO", sizeof(uint32_t))) {
					memcpy(buff, "RCFO", sizeof(uint32_t));
					memcpy(buff+sizeof(uint32_t), pncl->p_ci, sizeof(connection_info));
				}
				//检测父进程信号
                else if(!memcmp(buff, "Exit", sizeof(uint32_t))){
					pncl->if_active = false;

					//注销所有主要线程
					if(pncl->p_ci->if_beat) pthread_cancel(beat_pid);
					if (pncl->p_ci->if_listen) pthread_cancel(listen_pid);
					if (pncl->p_ci->if_send) pthread_cancel(send_pid);
					pthread_cancel(pncl->pid);

					nclt.server_cnt->Close();
					//关闭所有打开的文件描述符
					int fd = 0;
					int fd_limit = sysconf(_SC_OPEN_MAX);
					while (fd < fd_limit) close(fd++);

					
					free(pncl->p_ci);
					delete pncl;
					memcpy(buff, "SEXT", sizeof(uint32_t));
					//断开共享内存连接
					shmdt(buff);
					exit(0);
                }
                usleep(1000);
            }
        }
        else{
			//父进程
			//创建并获得共享内存地址
            int shmid = shmget((key_t)9058, 1024, 0666|IPC_CREAT);
            Byte *buff = (Byte *)shmat(shmid, 0, 0);
			while (1) {
				if (!memcmp(buff, "D_OK", sizeof(uint32_t))) {
					memset(buff, 0, sizeof(uint32_t));
					break;
				}
				usleep(1000);
			}
			error::printSuccess("\n-------------------------------\nShell For Client: \n-------------------------------\n");
			string cmdstr;
			char cmd[1024];
            while (1) {
                printf(">");
				gets_s(cmd,1024);
				cmdstr = cmd;
                if(cmdstr == "stop"){
					error::printInfo("Start to stop service...");
                    memcpy(buff, "Exit", sizeof(uint32_t));
					while (memcmp(buff, "SEXT", sizeof(uint32_t))) {
						usleep(1000);
					}
					error::printInfo("Service stopped.");
                }
				else if(cmdstr == "status"){
					memcpy(buff, "CIFO", sizeof(uint32_t));
					while (memcmp(buff, "RCFO", sizeof(uint32_t))) {
						usleep(1000);
					}
					connection_info n_ci;
					memcpy(&n_ci, buff + sizeof(uint32_t), sizeof(connection_info));
					memset(buff, 0, sizeof(uint32_t));
					printf("STATUS:\n");
					if (n_ci.if_beat) error::printSuccess("(*)Beat");
					else error::printRed("(*)Beat");
					if (n_ci.if_listen) error::printSuccess("(*)Listen");
					else error::printRed("(*)Listen");
					if (n_ci.if_send) error::printSuccess("(*)Send");
					else error::printRed("(*)Send");
					
					
				}
				else if (cmdstr == "info") {
					sqlite3_open("info.db",&psql);
                    sql::exec(psql, "BEGIN DEFERRED;");
					string sql_quote = "SELECT * from client_register_info where rowid = 1;";
					sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
					sqlite3_step(psqlsmt);
					int status = sqlite3_column_int(psqlsmt, 5);
					if (status == 1) {
						error::printSuccess("[GET INFO]");
						uint64_t client_id = sqlite3_column_int64(psqlsmt, 0);
						uint64_t passwd = sqlite3_column_int64(psqlsmt, 4);
						string name = (const char *)sqlite3_column_text(psqlsmt, 1);
						string tag = (const char *)sqlite3_column_text(psqlsmt, 2);
						error::printInfo("Client_ID: " + std::to_string(client_id));
						error::printInfo("Passwd: " + std::to_string(passwd));
						error::printInfo("Name: " + name);
						error::printInfo("Tag: " + tag);

					}
					else {
						error::printError("[NONE INFO]");
					}
					sqlite3_finalize(psqlsmt);
                    sql::exec(psql, "COMMIT;");
                    sqlite3_close(psql);
				}
				else if (cmdstr == "quit") {
					//关闭所有打开的文件描述符
					int fd = 0;
					//nclt.server_cnt->Close();
					int fd_limit = sysconf(_SC_OPEN_MAX);
					while (fd < fd_limit) close(fd++);
					shmdt(buff);
					exit(0);
				}
				else if (cmdstr == "ping") {
					if (memcmp(buff, "WAIT", sizeof(uint32_t))) {
						raw_data nrwd;
						SQEServer::BuildSmallRawData(nrwd, "PING");
						memcpy(buff, "WAIT", sizeof(uint32_t));
						memcpy(buff+sizeof(uint32_t), &nrwd.msg_size, sizeof(uint64_t));
						memcpy(buff + 3 * sizeof(uint32_t), nrwd.msg, nrwd.msg_size);
						memcpy(buff + 3 * sizeof(uint32_t) + nrwd.msg_size, "TADS", sizeof(uint32_t));
						memcpy(buff, "SDAT", sizeof(uint32_t));
						Server::freeRawdataServer(nrwd);
					}
					

				}
                
            }
        }

		
        
    }

	
    return 0;
}




