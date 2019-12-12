//
//  client.cpp
//  Net
//
//  Created by 胡一兵 on 2019/2/6.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "server.h"

pthread_mutex_t mutex_clt;

//客户端构造函数
Client::Client(int port, string send_ip,int send_port):socket(port),send_socket(send_ip,send_port){
    socket.UDPSetFCNTL();
    listen_port = port;
    sqlite3_stmt *psqlsmt;
    const char *pzTail;
    sqlite3_open("info.db", &psql);
    
    sql::exec(psql, "BEGIN DEFERRED;");
    string sql_quote = "select name,tag,msqes_key from client_info where rowid = 1;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    name = (const char *)sqlite3_column_text(psqlsmt, 0);
    tag = (const char *)sqlite3_column_text(psqlsmt, 1);
    sqe_key = (const char *)sqlite3_column_text(psqlsmt, 2);
    sqlite3_finalize(psqlsmt);
    sql::exec(psql, "COMMIT;");
    
}

//客户端请求接收守护进程
void *clientRequestDeamon(void *pvclt){
    clock_thread_info *pclt = (clock_thread_info *) pvclt;
    Client *pclient = (Client *) pclt->args;
    pclient->ProcessRequestListener();
    clockThreadFinish(pclt->tid);
    pthread_exit(NULL);
}

//客户端回复处理守护线程
void *clientRespondDeamon(void *pvclt){
    clock_thread_info *pclt = (clock_thread_info *) pvclt;
    Client *pclient = (Client *) pclt->args;
    int prm = 512;
    ssize_t tlen;
    char *str = nullptr;
    Addr taddr;
    do{
        tlen = pclient->socket.RecvRAW(&str,taddr);
        if(tlen > 0){
//            记录有效数据包
            if(Server::CheckRawMsg(str, tlen)){
                raw_data *ptrdt = new raw_data();
                Server::ProcessSignedRawMsg(str, tlen, *ptrdt);
                ptrdt->address = *(struct sockaddr_in *)taddr.RawObj();
                if (!memcmp(&ptrdt->info,"SPKT",sizeof(uint32_t))) {
                    packet npkt;
                    Server::Rawdata2Packet(npkt, *ptrdt);
                    
                    if(npkt.type == RESPOND_TYPE){
                        respond *pnres = new respond();
                        SQEServer::Packet2Respond(npkt, *pnres);
//                        加锁
                        if (pthread_mutex_lock(&mutex_clt) != 0) throw "lock error";
                        pclient->res_lst.push_back(pnres);
//                        解锁
                        pthread_mutex_unlock(&mutex_clt);
                    }
                    Server::freePcaketServer(npkt);
                }
                else{
                    
                }
                Server::freeRawdataServer(*ptrdt);
                delete ptrdt;
            }
        }
        free(str);

    }while (tlen && prm-- > 0);
    clockThreadFinish(pclt->tid);
    pthread_exit(NULL);
}

//客户端请求监听管理线程
void Client::ProcessRequestListener(void){
//    加锁
    if (pthread_mutex_lock(&mutex_clt) != 0) throw "lock error";
    for(auto &pres : res_lst){
        for(auto &lreq : req_lst){
            if(!lreq->active) continue;
//            检查回复号与请求号是否相同
            if(lreq->p_req->r_id == pres->r_id){
//                调用回调函数
                lreq->callback(pres,lreq->args);
                lreq->active = false;
            }
        }
        delete pres;
        pres = nullptr;
    }
    res_lst.remove_if([](auto &pres){return pres == nullptr;});
//    解锁
    pthread_mutex_unlock(&mutex_clt);
    
//    处理请求超时的情况
    for (auto lreq : req_lst) {
        if(!lreq->active) continue;
        if(lreq->clicks < lreq->timeout){
            lreq->clicks++;
//             重新发送数据包
            if(!(lreq->clicks % 2)){
                send_socket.SetSendSockAddr(*lreq->p_req->t_addr.Obj());
                SendRawData(&lreq->trwd);
            }
        }
        else{
            lreq->callback(NULL,lreq->args);
            delete lreq;
            lreq->active = false;
        }
    }
//    请求列表
    req_lst.remove_if([](auto &preq){return preq->active == false;});
}

//设置客户端守护时钟
void setClientClock(Client *pclient,int clicks){
    pthread_mutex_init(&mutex_clt, nullptr);
//    注册回复数据接收时钟
    clock_register *pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks;
    pncr->rawclick = clicks;
    pncr->func = clientRespondDeamon;
    pncr->arg = (void *)pclient;
    newClock(pncr);
    
//    注册请求监听处理时钟
    pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks+3;
    pncr->rawclick = clicks*2;
    pncr->func = clientRequestDeamon;
    pncr->arg = (void *)pclient;
    newClock(pncr);
    
}

//创建监听结构
void Client::NewRequest(request **ppreq,string send_ip,int send_port,string type, string data, bool if_encrypt){
    request *pnreq = new request();
    pnreq->type = type;
    pnreq->data = data;
    pnreq->t_addr.SetIP(send_ip);
    pnreq->t_addr.SetPort(send_port);
    pnreq->recv_port = listen_port;
    pnreq->if_encrypt = if_encrypt;
    *ppreq = pnreq;
}

//创建新的对服务器的请求的监听
void Client::NewRequestListener(request *preq, int timeout, void *args, void (*callback)(respond *,void *)){
    request_listener *pnrl = new request_listener();
    packet npkt;
    pnrl->active = true;
    pnrl->callback = callback;
    pnrl->timeout = timeout;
    pnrl->clicks = 0;
    pnrl->p_req = preq;
    pnrl->args = args;

    SQEServer::Request2Packet(npkt, *preq);
    Server::Packet2Rawdata(npkt, pnrl->trwd);
	//检查请求是否要求加密
    if(preq->if_encrypt == true){
        Server::EncryptRSARawMsg(pnrl->trwd, sqe_pbc);
        Server::SignedRawdata(&pnrl->trwd,"RPKT");
    }
    else{
        Server::SignedRawdata(&pnrl->trwd,"SPKT");
    }
	
    send_socket.SetSendSockAddr(*pnrl->p_req->t_addr.Obj());
    SendRawData(&pnrl->trwd);
    req_lst.push_back(pnrl);
}

request_listener::~request_listener(){
    Server::freeRawdataServer(trwd);
    delete p_req;
}

void Client::SetPublicKey(public_key_class &t_pbc){
    sqe_pbc = t_pbc;
}
