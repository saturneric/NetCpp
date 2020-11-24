//
// Created by 胡宇 on 2020/7/7.
//

#ifndef NET_TCP_SERVER_H
#define NET_TCP_SERVER_H


// 基础依赖
#include <project.h>

// 扩展依赖
#include "tcp.h"

namespace Net {

    class TCPServer {
    public:
        TCPServer(int port, int max_connection);

        ~TCPServer(){
            stop();
            close(fd);
        }

        void stop(){
            if(p_accept_manager_thread != nullptr)
                p_accept_manager_thread->interrupt();
            this->status = -1;
        }

        uint8_t readByte();

    private:

        int fd;
        int status = 0;
        struct sockaddr_in server_addr;
        std::queue<uint8_t> recv_buff;
        boost::mutex buff_mutex;
        boost::thread *p_accept_manager_thread;

        void cycle();

        static void accept_manager(TCPServer *server);

        static void accept(int fd, boost::mutex *buff_mutex, std::queue<uint8_t> *recv_buff, const int *status);

        void create_socket(int port);
    };

}

#endif //NET_TCP_SERVER_H
