//
//  addr.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.h"

Addr::Addr(string ip_addr, int port, bool ipv4){
    memset(&address, 0, sizeof(struct sockaddr_in));
    if(ipv4)
        address.sin_family = AF_INET;
    else
        address.sin_family = AF_INET6;
    address.sin_port = htons((uint16_t)port);
	address.sin_addr.s_addr = inet_addr(ip_addr.data());
    addr_size = sizeof(address);
}

Addr::Addr(struct sockaddr_in saddri){
    memset(&address, 0, sizeof(struct sockaddr_in));
    address = saddri;
}

Addr::Addr(){
    memset(&address, 0, sizeof(struct sockaddr_in));
}

Addr::Addr(const Addr &t_addr){
    address = t_addr.address;
    addr_size = t_addr.addr_size;
}


socklen_t *Addr::SizeP(void){
    return &addr_size;
}

socklen_t Addr::Size(void){
    return addr_size;
}

void Addr::SetSize(void){
    addr_size = sizeof(address);
}

void Addr::SetPort(int port){
    address.sin_port = htons((uint16_t)port);
    addr_size = sizeof(address);
}

void Addr::SetIP(string ip_addr){
	address.sin_addr.s_addr = inet_addr(ip_addr.data());
    addr_size = sizeof(address);
}

struct sockaddr_in *Addr::Obj(void){
    return &address;
}

struct sockaddr *Addr::RawObj(void){
    return (struct sockaddr *)&address;
}

void Addr::SetIpv4(void){
    address.sin_family = AF_INET;
    SetSize();
}

void Addr::SetIpv6(void){
    address.sin_family = AF_INET6;
    SetSize();
}

bool Addr::checkValidIP(string ipaddr){
    char temp[31];
    int a,b,c,d;
    
    if (sscanf(ipaddr.data(), "%d.%d.%d.%d ", &a, &b, &c, &d) == 4 && a >= 0 && a <= 255 && b >= 0 && b <= 255 && c >= 0 && c <= 255 && d >= 0 && d <= 255){
        sprintf(temp, "%d.%d.%d.%d", a, b, c, d); //把格式化的数据写入字符串temp
        if (!strcmp(temp, ipaddr.data())) return true; //success
        else return false;
    }
    else return false;
    return true;
}

void Addr::SetSockAddr(struct sockaddr_in tsi){
    address = tsi;
}
