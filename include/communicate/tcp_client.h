//
// Created by 胡宇 on 2020/7/9.
//

#ifndef NET_TCP_CLIENT_H
#define NET_TCP_CLIENT_H

#include <cstring>
#include <string>
#include <stdexcept>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <boost/thread/thread.hpp>

namespace Net {

    class TCPClient {
    public:
        TCPClient(const std::string &ip, int port);

        int sendData(const std::string &data);


    private:
        int fd{};
        struct sockaddr_in client_addr{};

        std::stringstream recv_buff;

        void create_socket_and_connection();

        void recv_cycle();

        static void recv_data(TCPClient *client);
    };

}


#endif //NET_TCP_CLIENT_H
