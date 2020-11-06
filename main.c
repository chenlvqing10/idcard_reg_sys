/*
 * 1.身份证:模块会自动发送帧头为BB的数据帧  格式:0xBB + 长度高8位+长度低8位 + 0x32 + N字节数据+1字节异或和校验  
 * 2.其它类型的卡片:会返回帧头为AA的数据帧  格式:0xAA + 1字节长度 + 0x01 + 1字节卡片类型 + n字节卡片UID
 * 3.卡片移开:返回 "AA 01 EA"
 * 4.可以根据不同的卡片类型 进行不同的功能设置
 * 5.解析身份证的方式：
 * 5.1.云解析身份证信息:需要连接服务器 网络编程
 * 5.2.本地解析身份证机读信息(文字信息+图片信息 并显示出来)
*/
#include "main.h"
#include "card.h"
#include "uart.h"
#include "idcard.h"
#include "server_socket.h"

/*全局变量*/
volatile int32_t cardread_status = CARDREAD_STATUS_NO;	/*读卡的状态 card reading status */
extern _St_IDCardData  idcard_info;						/*from "idcard.c"*/
extern uint8_t idcard_bitmap[1024];

int32_t main(int32_t argc, char *argv[]) 
{
	int32_t	ret;
	int32_t	card_type = NODETECT;

	/*初始化串口 init serial and set serial var*/
	int32_t	fd = uart_init();/*get file descriptor*/
	if(fd <0) {
		printf("初始化串口失败,请检查串口号\n");
		exit(-1);
	}
	else
		printf("初始化串口成功\n");

	/*循环处理模块发送的命令 loopping judge the command from idcard module*/
	while (1) {
		printf("start loop\n");
		sleep(10);//for debug
		/*1.检查是否有卡靠近模块(射频设备) 如果有卡片,则解析模块向串口输出的数据,返回卡片类型信息 check if having a card*/
		card_type = card_detect(fd,cardread_status);/* call fun from  "card.c" */

		/* 未检测到卡片 继续下一次循环检测 no detect card continue while loop*/
		if (card_type == NODETECT) {
			cardread_status = CARDREAD_STATUS_NO;
			printf("NO DETECT CARD\n");
			sleep(10);//for debug
			continue;
		}
		else {
			printf("HAVING DETECTED CARD\n");	
			tcflush(fd,TCIFLUSH);/* 检测到卡片 清空串口输入 detect card then clear the serial input queue */
			cardread_status = CARDREAD_STATUS_ING;/* 更新读卡状态  update card reading status  */

			/* 检查卡片是否很快被移除了 detect if the catd is moved */
			bool is_moved = is_cardMoved(fd);  /* call fun from "card.c" */
			if(is_moved == true) {
				printf("卡片被移除了\n");
				cardread_status = CARDREAD_STATUS_MOVED;
			}
			else {
				printf("卡片未被移除\n");
			}
			
			if(cardread_status == CARDREAD_STATUS_ING) {
				/* 读取卡片信息 reading card info*/
				ret = cardinfo_read(fd,cardread_status,card_type);/* call fun from "card.c" */
				//printf("读卡 ret = %d\n",ret);

				/* 读卡失败 重新读取 reading failed the continue while loop */
				if(ret < 0) {
					printf("读卡失败");
					cardread_status = CARDREAD_STATUS_FAILED;
					continue;
				}

				printf("读卡成功\n");	
				cardread_status = CARDREAD_STATUS_OK;/* 读卡成功 更新读卡状态 reading succuess then update card reading status*/
			}
			else if(cardread_status == CARDREAD_STATUS_MOVED) {
				printf("身份证已经移开 请重新放入身份证\n");
				cardread_status = CARDREAD_STATUS_NO;
				tcflush(fd,TCIOFLUSH);/* 刷新串口输入输出缓冲区 准备重新读卡 clear serial input and output queue then continue while loop*/
				continue;
			}

			/*3.卡片信息读取成功  解析卡片信息  本地身份证解析协议 
			 * reading success then anylysis the card info note:using local protocol
			 * if using cloud protocol then need to add association code
			*/
			cardinfo_anylys(fd,cardread_status,card_type);//解析完成之后  全局变量中

/* 如果开发板不能外接显示设备  需要发送到QT客户端去进行显示 if the development board has no diaplay fun then send data to QT client to diaplay*/
#ifdef __FCU2303__NOSCREEN__
	/*4.构建网络客户端  发送数据给QT程序客户端	check the data*/
	printf("main idcard_info.name = %s\n",idcard_info.name);
	printf("main idcard_info.gender = %s\n",idcard_info.gender);
	printf("main idcard_info.national = %s\n",idcard_info.national);
	printf("main idcard_info.birthday = %s\n",idcard_info.birthday);
	printf("main idcard_info.address = %s\n",idcard_info.address);
	printf("main idcard_info.idnumber = %s\n",idcard_info.idnumber);
	printf("main idcard_info.maker = %s\n",idcard_info.maker);
	printf("main idcard_info.start_date = %s\n",idcard_info.start_date);
	printf("main idcard_info.end_date = %s\n",idcard_info.end_date);

	for(int i=0;i<sizeof(idcard_bitmap);i++)
		printf("idcard_bitmap[%d] = 0x%02x ",i,idcard_bitmap[i]);
	printf("\nend.........................................................................\n");
	
	/* 不同的平台需要使用不同类型的解码动态库库解码身份证图片数据 
	 * Different platforms using different image decoding of ID card   
	*/
	/* 如果已经编译了AARCH64(ARM 64)的动态链接库则直接传递图片文件 否则将16进制数据传送出去
	 * if having LIB fro ARM64 platform then send a image file after decoing the date,or send tne Hex date 
	*/	
#ifdef		__FCU2303__ARM64__
			#ifdef      __HAVING__LIB__ARM64

			#else
			/* 创建本地服务器  发送数据到QT客户端  TCP协议  create local server then send date to QT client using TCP protocol */
			sendToQT(fd,&idcard_info,(char*)idcard_bitmap);
			#endif

#else		//__FCU2303__ARM32__
			#ifdef		__HAVING__LIB__ARM32
			char filename[100] = "./id.bmp";
			sendToQT(fd,&idcard_info,filename);
			#else

			#endif
#endif

#ifdef		__FCU2303__X64__
			#ifdef		__HAVING__LIB__X64
			char filename[36] = "./id.bmp";
			sendToQT(fd,&idcard_info,filename);
			#else

			#endif
#endif

#else/* 如果可以显示  则可以编译制作QT平台可以使用的动态库  if can diaplay then create lib file,QT call this lib's fun to get date,then disply*/

#endif
		}/* end having crad detect */
		
		/* server is out ,then set the card reading status NO*/
		if(cardread_status == CARDREAD_STATUS_MOVED)
		{
			printf("server is out because of id card moved\n");
			cardread_status = CARDREAD_STATUS_NO;
		}
	}/* end while */

	/* close serial */
	uart_close(fd);

	return 0;
}



