#include "server_socket.h"
extern volatile int32_t cardread_status;

/* 设置服务端IP,端口号 监听客户端连接 */
int listen_socket(const int port, int backlog)
{
#ifdef IPv4
	/* 创建IPv4流式套接字 */
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(0 > s) {
		printf("socket fail : %s\n", strerror(errno));
		return -1;
	}
	else {
		printf("socket create success!!!");
	}
#else
	/* 创建IPv6流式套接字 */
	int s = socket(AF_INET6, SOCK_STREAM, 0);
	if(0 > s) {
		printf("socket fail : %s\n", strerror(errno));
		return -1;
	}
	else {
		printf("socket = %d\n",s);
	}
#endif

#ifdef IPv4
	/* IPv4地址结构初始化 */
	struct sockaddr_in addr = {
		.sin_family = AF_INET,      //IPv4
		.sin_port   = htons(port),  //服务器端口
		.sin_addr   = {
			.s_addr = INADDR_ANY,   //本地任意IPv4地址
		},
		.sin_zero   = {0},
	};
#else
	/* IPv6地址结构初始化 */
	struct sockaddr_in6 addr = {
		.sin6_family    = AF_INET6, //IPv6
		.sin6_port  = htons(port),  //服务器端口
		.sin6_addr  = in6addr_any,  //本地任意IPv6地址
	};
#endif
	socklen_t len = sizeof(addr); //地址结构长度
	/* 输入时，告诉bind函数，地址结构的长度，输出时实际的长度 绑定IP和端口号*/
	if(0 > bind(s, (struct sockaddr *)&addr, len)) {
		printf("bind fail : %s\n", strerror(errno));
		goto ERR_SETP;
	}
	else {
		printf("server bind success!!");
	}

	//设置监听数
	if(0 > listen(s, backlog)) {
		printf("listen fail : %s\n", strerror(errno));
		goto ERR_SETP;
	}

	return s;

ERR_SETP:
	close(s);
	return -1;
}

/* 发送数据到QT客户端 */
int sendToQT(int fd,_St_IDCardData *idcard_info_st,char* image_data)
{
	/* for debuf print
	printf("name = %s  gender = %s national = %s  birthday = %s  address = %s  idnumber = %s maker = %s start_date = %s end_date =%s \n",
			idcard_info_st->name,idcard_info_st->gender,idcard_info_st->national,idcard_info_st->birthday,idcard_info_st->address,idcard_info_st->idnumber,
			idcard_info_st->maker,idcard_info_st->start_date,idcard_info_st->end_date);
	*/

	/* 设置服务器断端口号 */
	int port = SERVERPORT;

	/* 初始化监听套接字  监听客户端的连接 */
	int s = listen_socket(port, 10);
	if(0 > s) {
		goto NEXT_STEP;
	}
	printf("Wait for a client.\n");

	/* 地址结构说明 */
#ifdef IPv4
	struct sockaddr_in c_addr;
#else
	struct sockaddr_in6 c_addr;
#endif
	socklen_t c_len = sizeof(c_addr);
	/* 阻塞等连接：连接不来休眠等【函数不返回且不耗CPU时间】，直到连接来 */
	int rws = accept(s, (struct sockaddr *)&c_addr, &c_len);
	if(0 > rws) {
		perror("accept");
		goto NEXT_STEP1;
	}

#ifdef IPv4
	/* 保存字符串IP的缓存 */
	char strip[INET_ADDRSTRLEN]; /*for example【"255.255.255.255"】*/
	printf("A client [%s:%u] is entry.\n",
			/* 把IPv4地址转成字符串IP */
			inet_ntop(AF_INET, &c_addr.sin_addr, strip, INET_ADDRSTRLEN),
			ntohs(c_addr.sin_port));
#else
	/* 保存字符串IP的缓存 */
	char strip[INET6_ADDRSTRLEN];
	printf("A client [%s/64:%u] is entry.\n", strip,
			/* 把IPv6地址转成字符串IP */
			inet_ntop(AF_INET6, &c_addr.sin6_addr, strip, INET6_ADDRSTRLEN),
			ntohs(c_addr.sin6_port));
#endif
	//sleep(10);
	/* for debug print */
	//printf("name = %s  gender = %s national = %s  birthday = %s  address = %s  idnumber = %s maker = %s start_date = %s end_date =%s \n",
	//		idcard_info_st->name,idcard_info_st->gender,idcard_info_st->national,idcard_info_st->birthday,idcard_info_st->address,idcard_info_st->idnumber,
	//		idcard_info_st->maker,idcard_info_st->start_date,idcard_info_st->end_date);
	//for(int i=0;i<1024;i++)
	//	printf("image_data[%d]=0x%02x ",i,image_data[i]);
	//printf("\n");
	
	sleep(2);
	/* 循环发送数据给QT客户端处理 */
	while(1) {
		int ret;
		/* 将结构体转换为字符串  发送 */
		char snd_buf[SIZEINFO] = {0}; /* 300字节 */
		//printf("size =%ld\n",sizeof(*idcard_info_st));//288字节
		//sleep(5);
		memcpy(snd_buf,idcard_info_st,sizeof(*idcard_info_st)); /* 288 bytes */
		for(int i=0;i<sizeof(*idcard_info_st);i++)
			printf("snd_buf[%d] = %c\n",i,snd_buf[i]);

		ret = send(rws, snd_buf, sizeof(*idcard_info_st), MSG_NOSIGNAL);
		if(-1 == ret) {
			printf("snd content fail.\n");
			perror("send");
			goto NEXT_STEP2;
		}
		else {
			printf("info ret = %d\n",ret);
		}
		sleep(1); /* 隔两秒发送下一次的图片数据 wait 2s then send the image info */

		/* send the image info*/
#ifdef __FCU2303__ARM64__ /* RAM64平台 send 1024 bytes HEX data */
		char snd_image_buf[MAXSIZEBMP] = {0}; /* 1024 bytes */
		memcpy(snd_image_buf,image_data,MAXSIZEBMP);

		for(int i=0;i<sizeof(image_data);i++)
			printf("snd_image_buf[%d] = 0x%02x ",i,snd_image_buf[i]);
		printf("\n");
		//sleep(10);
		//
		ret = send(rws, snd_image_buf, sizeof(snd_image_buf), MSG_NOSIGNAL);
		if(-1 == ret) {
            printf("snd image content fail.\n");
            perror("send image");
            goto NEXT_STEP2;
        }
        else {
			printf("info ret = %d\n",ret);
        }

#else  /* send image file*/
		struct stat info;
		/* 读出文件属性[小文件] */
		if(0 > stat(image_data, &info)) {
			perror("stat");
			break;
		}
		printf("%s: %d\n", image_data, (int)info.st_size);

		/* 得到文件大小 (network order) 并从网络字节序转为本地字节序 */
		int len = htonl(info.st_size);

		/*阻塞版：要发送多少就必须发多少 发送文件大小；发送协议头：4字节整型，意义是文件大小 */
		if(4 != send(rws, &len, 4, MSG_NOSIGNAL)) {
			printf("snd head fail.\n");
			break;
		}

		/* 只读打开文件 */
		FILE *fp = fopen(image_data, "r");
		if(NULL == fp) {
			perror("fopen");
			break;
		}

		char buf[MAXSIZEBMP];
		/* 读一点文件内容，发一点  每次读1KB=1024 bytes */
		while(1) {
			/* 读出文件内容 */
			int num = fread(buf, 1, MAXSIZEBMP, fp);
			if(0 >= num) {
				/* 文件读完了，退出循环 */
				break;
			}
			/* 阻塞版发送函数：要发多少就必须发送多少
			 * 阻塞：IO时，IO的条件不满足，则休眠等；要发送数据，如果发不完则发送条件不满足，休眠等
			 * 直到能够发完返回
			 * 发送文件内容
			*/
			if(num != send(rws, buf, num, MSG_NOSIGNAL)) {
				printf("snd content fail.\n");
				fclose(fp);
				goto NEXT_STEP2;
			}
			printf("snd %s %dbytes done.\n", image_data, num);
		}/* end image while */
		/* 关闭文件 */
		fclose(fp);
#endif
		/* 检查卡片是否被移除 */
      bool is_moved = is_cardMoved(fd);  /* call fun from "card.c" */
      if(is_moved == true) {
          printf("卡片被移除了\n");
          cardread_status = CARDREAD_STATUS_MOVED;
		  break;
      }
      else {
          printf("卡片未被移除\n");
      }


	}/* end while */

NEXT_STEP2:
	close(rws);
NEXT_STEP1:
	close(s);
NEXT_STEP:
	printf("server is leave.\n");

	return 0;
}
