//
// Created by 胡宇 on 2020/7/7.
//

#include "communicate/tcp_server.h"
#include "debug_tools/print_tools.h"

extern int errno;

void Net::TCPServer::cycle() {
  boost::thread accept_manager_thread(TCPServer::connection_manager, this);
  this->p_conn_mgr_thrd = &accept_manager_thread;
  accept_manager_thread.detach();
}

void Net::TCPServer::connection_manager(Net::TCPServer *server) {
  Net::PrintTools::debugPrintSuccess("AcceptManager Started.");
  while (true) {
    int connect_fd = ::accept(server->fd, nullptr, nullptr);
    Net::PrintTools::debugPrintSuccess("New Connection.");

    if (connect_fd < 0)
      throw std::runtime_error(strerror(errno));
    else {
      boost::thread accept_thread(TCPServer::accept, connect_fd,
                                  &server->buff_mutex, &server->recv_buff,
                                  &server->status);
      accept_thread.detach();
    }
  }
}

void Net::TCPServer::accept(int fd, boost::mutex *buff_mutex,
                            std::queue<uint8_t> *recv_buff, const int *status) {
  Net::PrintTools::debugPrintSuccess("Try Getting Data From Connection.");
  uint8_t buff[1024];
  int len;
  std::vector<uint8_t> accept_buff;
  while ((len = recv(fd, buff, sizeof(buff), 0)) > 0 && *status == 0) {
    Net::PrintTools::debugPrintSuccess("Received.");
    for (int i = 0; i < len; ++i)
      accept_buff.push_back(buff[i]);
  }
  if (*status == 0) {
    buff_mutex->lock();
    for (unsigned char &i : accept_buff)
      recv_buff->push(i);
    buff_mutex->unlock();
  }
  close(fd);
  Net::PrintTools::debugPrintSuccess("Connection Closed.");
}

void Net::TCPServer::create_socket(int port) {
  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    throw std::runtime_error(strerror(errno));

  std::memset(&server_addr, 0, sizeof(struct sockaddr_in));
  this->server_addr.sin_family = AF_INET;
  this->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  this->server_addr.sin_port = htons(port);

  // Try binding the socket to the given port
  if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    // Close the socket created
    close(fd);
    throw std::runtime_error(strerror(errno));
  }

}

Net::TCPServer::TCPServer(uint16_t port, uint32_t max_connection=1024) {

  // Create a new socket binding certain port
  create_socket(port);

  if (listen(fd, max_connection) < 0) {
    // Close the socket created
    close(fd);
    throw std::runtime_error(strerror(errno));
  }

  // Start running the engine cycle
  cycle();
}

uint8_t Net::TCPServer::readByte() {
  uint8_t byte = '\0';
  buff_mutex.try_lock();
  if (!recv_buff.empty()) {
    byte = recv_buff.front();
    recv_buff.pop();
  }
  buff_mutex.unlock();
  return byte;
}
