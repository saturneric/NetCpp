//
// Created by 胡宇 on 2020/7/9.
//

#include "communicate/tcp_client.h"

void Net::TCPClient::recv_data(Net::TCPClient *client) {
    u_char buff[1024];
    int len;
    while ((len = recv(client->fd, buff, sizeof(buff), 0)) > 0) {
        client->recv_buff.write(reinterpret_cast<const char *>(buff), len);
    }
}

void Net::TCPClient::recv_cycle() {
    boost::thread recv_thread(TCPClient::recv_data,
                              this);
    recv_thread.detach();
}

void Net::TCPClient::create_socket_and_connection() {
    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::runtime_error("create socket failed.");
    }
    connect(fd, (struct sockaddr *) &client_addr, sizeof(client_addr));
}

int Net::TCPClient::sendData(const std::string &data) {
    create_socket_and_connection();
    recv_cycle();
    int len = send(fd, data.data(), data.size(), 0);
    close(fd);
    return len;
}

Net::TCPClient::TCPClient(const std::string &ip, int port) {
    std::memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    client_addr.sin_port = htons(port);
}
