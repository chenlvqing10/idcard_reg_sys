#ifndef __SERVER_SOCKET__
#define __SERVER_SOCKET__

#include <stdio.h>                 
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "idcard.h"
#include "type.h"
#include "card.h"
#define SERVERPORT  66691
#define IPv4
#define SIZEINFO    300
#define MAXSIZEBMP  1024

int listen_socket(const int port, int backlog);/* 监听客户端套接字 */
int sendToQT(int fd,_St_IDCardData *idcard_info_st,char* image_data); /* 发送数据到QT服务端 */

#endif
