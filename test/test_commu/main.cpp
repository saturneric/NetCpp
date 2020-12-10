//
// Created by 胡宇 on 2020/7/9.
//

#include <cstdio>
#include <unistd.h>

#include "communicate/tcp_client.h"
#include "communicate/tcp_server.h"
#include "debug_tools/print_tools.h"

using namespace Net;

int main(int argc, char *argv[]) {
  PrintTools::debugPrintSuccess("TCPServer Started.");
  TCPServer server(9048, 100);
  while (true) {
    std::putchar(server.readByte());
    usleep(1e3);
  }
  return 0;
}
