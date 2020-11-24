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

// 
// Provide stable, reliable, easy-to-use, many-to-one TCP communication.
// Requirement:
//  1. It must be bound to an available port before use.
// Features:
//  1. Support communication concurrency based on multithreading.
//
class TCPServer {
public:

  // 
  // Please provide an avaliable port(1~65535) first
  // The default maximum connection limit is 1024
  //
  TCPServer(uint16_t port, uint32_t max_connection);

  ~TCPServer() {
    stop();
    close(fd);
  }

  void stop() {
    if (p_conn_mgr_thrd != nullptr)
      p_conn_mgr_thrd->interrupt();
    this->status = -1;
  }

  uint8_t readByte();

private:
  int fd;
  int status = 0;
  struct sockaddr_in server_addr;
  std::queue<uint8_t> recv_buff;
  boost::mutex buff_mutex;
  boost::thread *p_conn_mgr_thrd;

  void cycle();

  static void connection_manager(TCPServer *server);

  static void accept(int fd, boost::mutex *buff_mutex,
                     std::queue<uint8_t> *recv_buff, const int *status);

  // 
  // Using the standard UNIX way to create a new socket
  // Requirement:
  //  the given  port need to be free for the binding
  // Return:
  //  a new and initialized socket which can be given to listen()
  void create_socket(int port);
};

} // namespace Net

#endif // NET_TCP_SERVER_H
