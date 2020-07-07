//
// Created by 胡宇 on 2020/7/7.
//

#include "communicate/tcp_server.h"

void Net::TCPServer::cycle() {
    boost::thread accept_manager_thread(TCPServer::accept_manager, this);
    this->p_accept_manager_thread = &accept_manager_thread;
    accept_manager_thread.join();
}

void Net::TCPServer::accept_manager(Net::TCPServer *server) {
    while(true){
        int connect_fd = ::accept(server->fd, nullptr, nullptr);
        if(connect_fd == -1) throw std::runtime_error("accept tcp connection error");
        else{
            boost::thread accept_thread(TCPServer::accept, connect_fd, &server->buff_mutex, &server->recv_buff, &server->status);
            accept_thread.detach();
        }
    }
}

void Net::TCPServer::accept(int fd, boost::mutex *buff_mutex, std::stringstream *recv_buff, const int *status) {
    uint8_t buff[1024];
    int len;
    std::vector<uint8_t> accept_buff;
    while((len = recv(fd, buff, sizeof(buff), 0)) > 0 && *status == 0){
        for(int i = 0; i < len; ++i) accept_buff.push_back(buff[i]);
    }
    if(*status == 0) {
        buff_mutex->lock();
        recv_buff->write(reinterpret_cast<const char *>(accept_buff.data()), accept_buff.size());
        buff_mutex->unlock();
    }
    close(fd);
}

void Net::TCPServer::create_socket(int port) {
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(!~fd) throw std::runtime_error("could not create socket file.");

    std::memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
}

Net::TCPServer::TCPServer(int port, int max_connection) {

    create_socket(port);

    if(bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
        throw std::runtime_error("bind port failed.");

    if(listen(fd, max_connection) == -1)
        throw std::runtime_error("listen socket failed.");


    cycle();
}

uint8_t Net::TCPServer::readByte() {
    return recv_buff.get();
}
