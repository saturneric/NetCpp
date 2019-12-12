//
//  server.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/16.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef server_h
#define server_h

#include "type.h"

#include "clock.h"
#include "net.h"
#include "cpart.h"
#include "cthread.h"
#include "sqlite3.h"
#include "rsa.h"
#include "rng.hpp"
#include "aes.h"
#include "sha1.h"

using namespace SQLite;
using std::shared_ptr;
using std::unique_ptr;

class Server;

//外来数据包解析结构
struct compute_result{
    string name;
    vector<void *> *args_in;
    vector<void *> *args_out;
    vector<int> *fargs_in;
    vector<int> *fargs_out;
};

//请求数据包
struct request {
//    匹配id
    uint64_t r_id;
//    类型
    string type;
//    数据
    string data;
//    接收端口
    uint32_t recv_port;
    
//    json结构
    Document req_doc;
    StringBuffer doc_str;
    
//    标记是否为加密请求
    bool if_encrypt;
    Addr t_addr;
    request();
    void Json2Data(void);
    void JsonParse(string data_from);
};

//加密端对端报文
struct encrypt_post{
//    注册客户端id
    uint64_t client_id;
//    目标地址信息
    Addr t_addr;
//    匹配id
    uint64_t p_id;
//    类型
    uint32_t type;
//    内容
    Byte *buff = nullptr;
//    内容长度
    uint32_t buff_size = 0;
	Document edoc;
	StringBuffer sb;
	bool Parse(string json);
	void SelfParse(void);
	void GetJSON(string &json);

    void SetBuff(Byte *buff, uint32_t size);
    void FreeBuff(void);
    ~encrypt_post(void);
	void InitNew(uint64_t client_id, Addr t_addr, const char *type);
};

//回复数据包
struct respond {
    uint64_t r_id;
    string type;
    Byte *buff = nullptr;
    uint32_t buff_size;
    Addr t_addr;
    void SetBuff(Byte *buff, uint32_t size);
    ~respond();
};

//通用数据包类
class packet{
public:
//    数据包类型
    unsigned int type;
    struct sockaddr_in address;
//    记录块的大小及内容所在的内存地址
    vector<pair<unsigned int, void *>> buffs;
    void AddBuff(const void *pbuff, uint32_t size);
    bool if_encrypt = false;
    ~packet();
};

//带标签的二进制串管理结构
class raw_data{
public:
//    二进制串
    unsigned char *data = NULL;
    unsigned long size = 0;
    uint64_t r_id;
//    标签
    uint32_t head, tail;
    uint32_t info;
//    信息串
    char *msg = NULL;
	unsigned long msg_size = 0;
//    来源ip地址
    struct sockaddr_in address;
//    用简单字符串直接出适合
    void setData(string str){
        data = (unsigned char *)malloc(str.size());
        size = str.size();
        memcpy(data, str.data(),str.size());
    }
    raw_data();
};

//请求监听管理结构
struct request_listener{
    void (*callback)(respond *,void *args);
    request *p_req;
    uint32_t timeout;
    uint32_t clicks;
    raw_data trwd;
    bool active;
    void *args;
    ~request_listener();
};

struct server_info{
    string tag;
    string name;
    string msqes_ip;
    int msqes_prot;
    string key;
};

struct aes_key256{
    uint64_t key[4];
    uint64_t iv[4];
//    生成新的随机密钥
    aes_key256();

    void MakeIV(void);
//    获得初始化向量
    const uint8_t *GetIV(void);
    const uint8_t *GetKey(void);
    
};

//UDP分包
struct net_box{
    uint16_t idx;
    uint16_t cnt;
    uint32_t head;
    uint32_t tail;
    uint64_t b_id;
    void *data = nullptr;
    uint16_t data_size = 0;
    
    UByte *send_data = nullptr;
    uint16_t sdt_size = 0;
    void set(void *pbuff, uint16_t pbsize);
    void build(void);
    void FreeNetBox(void);
    net_box();
    ~net_box();
};

//UDP分包监听结构
struct box_listener{
    uint64_t b_id;
//    生命
    int32_t clicks;
//    应该接收的分包数量
    uint16_t cnt;
//    接收到的分包数量
    uint16_t nbn;
	//分包来源地址
	sockaddr_in address;
//    储存接收到的分包的动态数组
    net_box **boxs;
//    合并分包成RawData
    void TogtRawData(raw_data &trdt);
//    释放动态数组所关联的所有内存
    void free_boxs(void);
};

//注册客户端管理
struct client_register{
//    客户端id
    uint64_t client_id;
//    通信密钥
    aes_key256 key;
    string name;
    string tag;
//    服务器资源租用时间
    uint32_t click;
//    认证口令
    uint64_t passwd;
//    目标地址信息
    Addr t_addr;
//    守护线程ID
    pthread_t tid;
	sqlite3 *psql;
};

struct client_listen{
    bool if_get;
    bool if_connected = true;
    pthread_t pid;
    SocketTCPClient *ptcps;
    encrypt_post *pcryp;
    client_register *pcltr;
};

struct connection_info {
	bool if_listen = false;
	bool if_beat = false;
	bool if_send = false;
};

struct connection_listener{
    int data_sfd;
    Addr client_addr;
    aes_key256 key;
    pthread_t pid = 0;
	void *father_buff = nullptr;
	SocketTCPCServer *server_cnt = nullptr;
	bool if_active = true;
	bool *pif_atv = nullptr;
	void *write_buff = nullptr;
	struct connection_info *p_ci = nullptr;
	pthread_t *beat_pid  = nullptr, *listen_pid = nullptr, *send_pid = nullptr;
	sqlite3 *psql;
};

//通用服务器类
class Server{
protected:
//    缓存通用数据包
    list<packet *> packets_in;
//    缓存带标签的二进制串管理结构
    list<raw_data *> rawdata_in;
    map<uint64_t, client_register *> rids;
//    输出的数据包列表
    list<packet *> packets_out;
    map<uint64_t,box_listener *> boxls;
    struct server_info tsi;
    unique_ptr<SQLite::Database> db;
//    服务器公私钥
    public_key_class pkc;
    private_key_class prc;
public:
//    服务器类的接收套接字对象与发送套接字对象
    SocketUDPServer socket;
    SocketUDPClient send_socket;
    int packet_max = 1024;
    Server(int port = 9048, string send_ip = "127.0.0.1",int send_port = 9049);
    
//    重新设置服务器的发送端口
    void SetSendPort(int port);
//    重新设置服务器的发送IP地址
    void SetSendIP(string ip_addr);
//    将结构数据包转换成二进制串
    static void Packet2Rawdata(packet &tpkt, raw_data &rdt);
//    将通用二进制串转换为通用数据包
    static void Rawdata2Packet(packet &tpkt, raw_data &trdt);
//    释放二进制串占用的空间
    static void freeRawdataServer(struct raw_data &trdt);
//    释放通用数据包包占用
    static void freePcaketServer(struct packet tpkt);
    
//    释放计算结果包占用的空间
    static void freeCPURServer(struct compute_result tcpur);
//    给二进制串贴上识别标签
    static void SignedRawdata(struct raw_data *trdt,string info);
//    发送已经贴上标签的二进制串
    int SentRawdata(struct raw_data *trdt);
//    检查消息串是否为一个贴上标签的二进制串
    static bool CheckRawMsg(char *p_rdt, ssize_t size);
//    处理一个已贴上标签的原始二进制串，获得其包含的信息
    static void ProcessSignedRawMsg(char *p_rdt, ssize_t size, raw_data &rdt);
//    解码已加密的原始二进制串
    static void DecryptRSARawMsg(raw_data &rdt, private_key_class &pkc);
//    编码原始二进制串
    static void EncryptRSARawMsg(raw_data &rdt, public_key_class &pkc);
//    检查是否为UDP分包
    static bool CheckNetBox(char *p_nb, ssize_t size);
//    将二进制信息转换成UDP分包
    static void ProcessNetBox(net_box &tnb, Byte *p_data);
//    服务器守护线程
    friend void *serverDeamon(void *psvr);
//    分包处理守护线程
    friend void *boxProcessorDeamon(void *pvcti);
//    处理RawData
    void ProcessRawData(void);
    void ProcessSendPackets(void);
    void CleaningBoxs(void);
    
    
};

//计算节点服务器类
class CNodeServer:public Server{
    vector<compute_result> cpurs_in;
public:
//    将计算结果包转化为结构数据包
    static packet CPURS2Packet(compute_result tcpur);
//    将结构数据包转化为计算结果包
    static compute_result Packet2CPUR(packet *tpkt);
};


class SQEServer: public Server{
protected:
//    请求数据包
    list<request *> req_list;
//    注册客户端管理
    map<uint64_t,client_register *> client_lst;
//    加密端对端报文
    list<encrypt_post *>post_lst;
	//服务器名
	string name;
public:
    SQEServer(int port = 9048);
    void ProcessPacket(void);
    void ProcessRequset(void);
    static void Packet2Request(packet &pkt, request &req);
    static void Request2Packet(packet &pkt, request &req);
    static void Respond2Packet(packet &pkt, respond &res);
    static void Packet2Respond(packet &pkt, respond &res);
    static void BuildBeatsRawData(raw_data &rwd);
    static void BuildSmallRawData(raw_data &rwd, const char *info);
    static void Post2SignedRawData(void *buff, uint32_t buff_size, const char *info, aes_key256 &key, raw_data &rw);
	static void Post2SignedRawData(encrypt_post &ecyp, aes_key256 &key, raw_data &rw);
    static void SignedRawData2Post(raw_data &rwd, encrypt_post &pst, aes_key256 &key);
    static void Post2Packet(packet &pkt, encrypt_post &pst, aes_key256 &key);
    static void Packet2Post(packet &pkt, encrypt_post &pst, aes_key256 &key);
    static void GetPostInfo(packet &pkt, encrypt_post &pst);
	static void SendConnectionInfo(SocketTCPClient *pcnt_sock, string type);
};

//通用客户端类
class Client{
//    请求监听列表
    list<request_listener *> req_lst;
    list<raw_data *> rwd_lst;
    list<encrypt_post *> ecryp_lst;
//TCP模式下有效二进制段列表
	list<raw_data *> rwd_tcp;
//    回复处理列表
    list<respond *> res_lst;
//    请求监听端口
    uint16_t listen_port;
    SocketUDPServer socket;
    SocketUDPClient send_socket;
//    与服务器建立的稳定链接
    SocketTCPCServer *server_cnt;
//    广场服务器通信公钥
    public_key_class sqe_pbc;
//    报文密钥
    aes_key256 post_key;
//    客户端名与标签
    string name,tag;
//    广场服务器服务密钥
    string sqe_key;
//    数据库
    sqlite3 *psql;
public:
//    构造函数(send_port指的是发送的目标端口)
    Client(int port = 9050, string send_ip = "127.0.0.1",int send_port = 9049);
//    处理请求监听
    void ProcessRequestListener(void);
//    新的请求
    void NewRequest(request **ppreq,string send_ip,int send_port,string type, string data, bool if_encrypt = false);
//    新的请求监听
    void NewRequestListener(request *preq, int timeout, void *args, void (*callback)(respond *, void *));
//    设置公钥
    void SetPublicKey(public_key_class &t_pbc);
//    设置AES密钥
    void SetAESKey(aes_key256 &key);
//    发送RawData
    void SendRawData(raw_data *trdt);
//    友元回复接受守护进程
    friend void *clientRespondDeamon(void *);
//    友元客户端控制器
    friend int client(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
};

//设置服务器守护线程的时钟
void setServerClock(Server *psvr, int clicks);
//设置广场服务器守护线程的时钟
void setServerClockForSquare(SQEServer *psvr, int clicks);
//服务器接收数据包守护线程
void *serverDeamon(void *psvr);
//服务器处理原始数据守护线程
void *dataProcessorDeamon(void *pvcti);
//UDP分包监听守护进程
void *boxProcessorDeamon(void *pvcti);
//UDP分包监听清理守护进程
void *boxsCleaningProcessorDeamon(void *pvcti);
//广场服务器处理数据包守护线程
void *packetProcessorDeamonForSquare(void *pvcti);
//广场服务器处理请求守护线程
void *requestProcessorDeamonForSquare(void *pvcti);
//服务器发送数据包守护线程
void *sendPacketProcessorDeamonForSquare(void *pvcti);


//设置客户端请求监听守护时钟
void setClientClock(Client *pclient,int clicks);
//客户端请求监听守护线程
void *clientRequestDeamon(void *pvclt);
//客户端回复接收守护线程
void *clientRespondDeamon(void *pvclt);
//客户端待机守护线程
void *clientWaitDeamon(void *pvclt);

#endif /* server_h */
