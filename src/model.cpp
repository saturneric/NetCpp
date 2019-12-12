//
//  model.cpp
//  Net
//
//  Created by 胡一兵 on 2019/2/8.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "instruct.h"

extern int if_wait;

bool config_search(vector<string> &configs,string tfg){
    for(auto config : configs){
        if(config == tfg) return true;
    }
    return false;
}

void getSQEPublicKey(respond *pres,void *args){
    if(pres != nullptr){
        public_key_class *npbc = (public_key_class *)pres->buff;
        sqlite3 *psql = (sqlite3 *)args;
        sqlite3_stmt *psqlsmt;
        const char *pzTail;
        sql::exec(psql, "BEGIN;");
        string sql_quote = "update client_info set msqes_rsa_public = ?1 where rowid = 1;";
        sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
        sqlite3_bind_blob(psqlsmt, 1, npbc, sizeof(public_key_class), SQLITE_TRANSIENT);
        sqlite3_step(psqlsmt);
        sqlite3_finalize(psqlsmt);
        sql::exec(psql, "COMMIT;");
        if_wait = 0;
    }
    else if_wait = -1;
}

void loginSQECallback(respond *pres, void *args) {
	if (pres != nullptr) {
		string resjson = string(pres->buff, pres->buff_size);
		Document resdoc;
		resdoc.Parse(resjson.data());
		string status = resdoc["status"].GetString();
		if (status == "ok") {
			error::printSuccess("login succeed.");
			if_wait = 0;
		}
		else {
			error::printError("login failed.");
			if_wait = -1;
		}
	}
	else {
		if_wait = -1;
		printf("Request timeout.\n");
	}
}

void registerSQECallback(respond *pres,void *args){
    if(pres != nullptr){
        string resjson = string(pres->buff,pres->buff_size);
        Document resdoc;
        resdoc.Parse(resjson.data());
        string status = resdoc["status"].GetString();
        if(status == "ok"){
			sqlite3 *psql = (sqlite3 *)args;
			try {
				//创建客户端描述信息数据表
				sql::table_create(psql, "client_register_info", {
					{"client_id","INT"},
					{"name","TEXT"},
					{"tag","TEXT"},
					{"key","NONE"},
					{"passwd","INT"},
					{"status","INT"},
					});
			}
			catch (const char *errstr) {
				string errinfo = errstr;
				if (errinfo == "fail to create table") {
					error::printInfo("Table is already created.");

					sqlite3_stmt *psqlsmt;
					const char *pzTail;
					string sql_quote = "delete from client_register_info;";
					sql::exec(psql, "BEGIN;");
					sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
					sqlite3_step(psqlsmt);
					sqlite3_finalize(psqlsmt);
                    sql::exec(psql, "COMMIT;");
				}
			}
            if_wait = 0;
        }
        else{
			error::printError("register failed.");
            if_wait = -1;
        }
    }
    else{
        if_wait = -1;
        printf("Request timeout.\n");
    }
}

//客户端连接管理守护线程
void *connectionDeamon(void *args){
    connection_listener * pcntl = (connection_listener *)args;
    string first_data;
    //printf("Start Listen Connection From Server.\n");
    char *buff = nullptr;
    Addr t_addr;
    ssize_t size = 0;
    SocketTCPCServer ntcps;
    ntcps.SetDataSFD(pcntl->data_sfd);
    ntcps.SetClientAddr(pcntl->client_addr);
//    获得连接的类型是长链还是断链
    size = ntcps.RecvRAW_SM(&buff, t_addr);
    raw_data *pnrwd = new raw_data();

//    检测连接是长连接还是短连接
    bool if_sm = true;
	string dget = "DGET";
    if(Server::CheckRawMsg(buff, size)){
        Server::ProcessSignedRawMsg(buff, size, *pnrwd);
        if(!memcmp(&pnrwd->info, "LCNT", sizeof(uint32_t))){
			//接收长连接
            if_sm = false;
        }
        else if(!memcmp(&pnrwd->info, "SCNT", sizeof(uint32_t))){
			//接收短连接
            if_sm = true;
			ntcps.SendRespond(dget);
        }
        else if(!memcmp(&pnrwd->info, "CNTL", sizeof(uint32_t))){
			//发送长连接
			if_sm = false;
			pcntl->p_ci->if_listen = true;
			*pcntl->listen_pid = pcntl->pid;
			pcntl->write_buff = pcntl->father_buff;
			while (1) {
				if (*pcntl->pif_atv == false) {
					close(pcntl->data_sfd);
					pcntl->p_ci->if_listen = false;
					delete pcntl;
					pthread_exit(NULL);
				}
				if (!memcmp(pcntl->write_buff, "SDAT", sizeof(uint32_t))) {
					uint32_t nsrwd_size = 0;
					Byte buff[BUFSIZ];
					memcpy(&nsrwd_size, ((Byte *)pcntl->write_buff + sizeof(uint32_t)), sizeof(uint32_t));
					if (!memcmp((Byte *)pcntl->write_buff + 3 * sizeof(uint32_t) + nsrwd_size, "TADS", sizeof(uint32_t))) {
						memcpy(buff, (Byte *)pcntl->write_buff + 3 * sizeof(uint32_t), nsrwd_size);
						send(pcntl->data_sfd, buff, nsrwd_size, 0);
					}
					else error::printError("buffer error.");
					memset(pcntl->write_buff, 0, sizeof(uint32_t));
				}
				usleep(1000);
				
			}
        }
        else{
			//断开无效连接
            printf("Connection Info Illegal.\n");
			delete pnrwd;
			close(pcntl->data_sfd);
			delete pcntl;

            pthread_exit(NULL);
        }
        
    }
    else{
        printf("Connection Illegal.\n");
        delete pnrwd;
        pthread_exit(NULL);
    }
	Server::freeRawdataServer(*pnrwd);
    delete pnrwd;
    
    while (1) {
		if (*pcntl->pif_atv == false) {
			close(pcntl->data_sfd);
			delete pcntl;
			break;
		}
		//区分长连接与短连接
        if(if_sm) size = ntcps.RecvRAW(&buff, t_addr);
        else size = ntcps.RecvRAW_SM(&buff, t_addr);
        if(size > 0){
			raw_data *pnrwd = new raw_data();
			packet *nppkt = new packet();
			encrypt_post *pncryp = new encrypt_post();
            if(Server::CheckRawMsg(buff, size)){
                Server::ProcessSignedRawMsg(buff, size, *pnrwd);
				//获得端对端加密报文
                if(!memcmp(&pnrwd->info, "ECYP", sizeof(uint32_t))){
                    Server::Rawdata2Packet(*nppkt, *pnrwd);
                    SQEServer::Packet2Post(*nppkt, *pncryp, pcntl->key);
					//获得注册信息反馈报文
                    if(!memcmp(&pncryp->type, "JRES", sizeof(uint32_t))){
						//自我解析
						pncryp->SelfParse();
						printf("Register Status: ");
                        if(pncryp->edoc["status"].GetString() == string("ok")){
							uint64_t client_id = pncryp->edoc["client_id"].GetInt64();
							uint64_t passwd = pncryp->edoc["passwd"].GetInt64();
							sqlite3_stmt *psqlsmt;
							const char *pzTail;
							string sql_quote = "update client_register_info set client_id = ?1,passwd = ?2,status = 1 where rowid = 1;";
							sqlite3_prepare(pcntl->psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
							sqlite3_bind_int64(psqlsmt, 1, client_id);
							sqlite3_bind_int64(psqlsmt, 2, passwd);
							if (sqlite3_step(psqlsmt) == SQLITE_OK) {
								error::printSuccess("Succeed");
							}
							else {
								error::printRed("Failed");
								sql::printError(pcntl->psql);
							}
							sqlite3_finalize(psqlsmt);



							error::printInfo("\nStart Command Line Tools...\n");
							//进入客户端管理终端
							memcpy(pcntl->father_buff,"D_OK", sizeof(uint32_t));
                        }
                    }
					//管理指令连接
					else if (!memcmp(&pnrwd->info, "JCMD", sizeof(uint32_t))) {
						//来自管理员的命令

					}
                }
				//心跳连接
                else if(!memcmp(&pnrwd->info, "BEAT", sizeof(uint32_t))){
					
					if (!pcntl->p_ci->if_beat) {
						pcntl->p_ci->if_beat = true;
						*pcntl->beat_pid = pcntl->pid;
						
					}
                }
                Server::freeRawdataServer(*pnrwd);
                Server::freePcaketServer(*nppkt);
            }
            free(buff);
			delete pnrwd;
			delete pncryp;
			delete nppkt;
        }
        usleep(1000);
    }
    
    pthread_exit(NULL);
}


void *clientServiceDeamon(void *arg) {
	connection_listener *pclst = (connection_listener *)arg;
	
	while (1) {
		if (pclst->if_active == false) {
			break;
		}
		//接受新连接
		pclst->server_cnt->Accept();

		//构造连接守护子线程
		connection_listener *pncl = new connection_listener();
		pncl->client_addr = pclst->client_addr;
		pncl->data_sfd = pclst->server_cnt->GetDataSFD();
		//设置超时
		struct timeval timeout = { 3,0 };
		setsockopt(pncl->data_sfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));

		pncl->key = pclst->key;
		pncl->father_buff = pclst->father_buff;
		pncl->pif_atv = &pclst->if_active;
		pncl->p_ci = pclst->p_ci;
		pncl->beat_pid = pclst->beat_pid;
		pncl->listen_pid = pclst->listen_pid;
		pncl->send_pid = pclst->send_pid;
		pncl->psql = pclst->psql;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&pncl->pid, &attr, connectionDeamon, pncl);
		pthread_attr_destroy(&attr);
		usleep(1000);
	}
    pthread_exit(NULL);
}

void gets_s(char *buff, uint32_t size) {
	char ch;
	uint32_t i = 0;
	while ((ch = getchar()) != '\n' && i < (size - 1)) {
		buff[i++] = ch;
	}
	buff[i] = '\0';
}
