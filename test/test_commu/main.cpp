//
// Created by 胡宇 on 2020/7/9.
//

#include <unistd.h>
#include <cstdio>

#include "debug_tools/print_tools.h"
#include "communicate/tcp_server.h"
#include "communicate/tcp_client.h"

using namespace Net;

int main(int argc, char *argv[]){
    if(fork() == 0) {
        PrintTools::debugPrintSuccess("Child Started.");
        TCPClient client("127.0.0.1", 9048);
        for(int i = 0; i < 32; i++, usleep(1e4))
            client.sendData("Hello");
        PrintTools::debugPrintSuccess("Child Exited.");
    }
    else{
        PrintTools::debugPrintSuccess("Father Started.");
        TCPServer server(9048, 100);
        while (true){
            std::putchar(server.readByte());
            usleep(1e3);
        }
    }
    return 0;
}