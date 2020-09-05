//
// Created by 胡宇 on 2020/7/7.
//

#ifndef NET_TCP_SERVER_H
#define NET_TCP_SERVER_H

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <string>
#include <queue>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>


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
