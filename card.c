#include "card.h"
#include "uart.h"
#include "idcard.h"

volatile static uint8_t cardtype = NODETECT; /* card type */
extern volatile int32_t cardread_status;     /* reading card status */
static uint8_t info[256] = {0};              /* idcard word info hex data */
uint8_t idcard_bitmap[1024] = {0};           /* idcard image info hex data */

/*reading the frame head and return*/
uint8_t get_frame_head(int32_t fd)
{
	/* 读取串口的第一个字节(帧头) */
	uint8_t  frame_head = ' ';
	int32_t ret = uart_read(fd,&frame_head,1,1000);
	if(ret < 0) {
		printf("帧头读取失败,目前串口未应答\n");
		printf("模块返回的帧头是:0x%2X\n",frame_head);
		frame_head = '\xFF';
	}
	else {
		printf("模块返回的帧头是:0x%2X\n",frame_head);
	}

	return frame_head;
}

/* get the card type and return */
uint8_t get_cardtype(int32_t fd)
{
	/* 读取串口返回的卡片类型 */
	uint8_t* frameinfo = NULL;
	int32_t ret = uart_read(fd,frameinfo,4,1000);
	if(ret < 0) {
		printf("从串口读取四字节数据失败\n");
		cardtype = NODETECT;
	}
	else {
		cardtype = frameinfo[3];/* 得到卡片类型的字节数据 */
		printf("模块返回的卡片类型是:0x%2X",cardtype);
	}

	return cardtype;
}

/* 检测不同类型的卡片  detect card type and return*/
int32_t  card_detect(int32_t fd,int32_t cardread_status)
{
	/* 如果前期已经检测到了卡片,则直接返回卡片类型 if card status is not no/moved then directly return the cardtype */
	if(cardread_status != CARDREAD_STATUS_NO)
		return cardtype;

	/* 1.读取模块通过串口返回的帧头 reading the frame head */
	uint8_t frame_head_res = get_frame_head(fd);
	if(frame_head_res == NODETECT)
		return cardtype;
	else {
		if(!memcmp(&frame_head_res,FRAME_HEAD,1)) {/*"AA"  非身份证卡片 */
			cardtype = get_cardtype(fd);
		}
		else if(!memcmp(&frame_head_res,FRAME_OL_HEAD,1)) { /*"BB" 身份证卡片 */
			cardtype = ID_CARD;
		}
		else { /* 乱码  可能硬件问题  接线电平问题 */
			cardtype = NODETECT;
		}
	}

	return cardtype;
}

/* 检测卡片是否已经被移除 detect is card moved*/
bool is_cardMoved(int32_t fd)
{
	//printf("call %s line = %d\n",__FUNCTION__,__LINE__);
	uint8_t buf[3];
	int32_t ret = read(fd,buf,3); /* 读取三字节数据 reading 3 bytes date */
	
	/* debug print */
	for(int i=0;i<3;i++)
		printf("moved buf[%d] = 0x%02x",i,buf[i]);
	
	if(ret < 0) { 
		printf("%s  串口读取失败\n",__FUNCTION__);
		return false;
	}
	
	if(!memcmp(&buf[0],"\x00",1)) {
		printf("%s 输入串口无数据\n",__FUNCTION__);
		return false;
	}
	
	if(!memcmp(&buf[1],"\xAA",1)) {
		printf("%s 非卡片移动帧\n",__FUNCTION__);
		return false;
	}

	if(!memcmp(buf,CARD_MOVEDD,3))
		ret = true;
	
	//printf("卡片移除判断函数结束\n");
	return ret;
}


/* 读取卡片信息 read card infomation*/
int32_t  cardinfo_read(int32_t fd,int32_t cardread_status,int32_t card_type)
{
	int ret;
	switch(card_type)                                                                                                      
	{
		case NODEFINE:
			printf("unkown card type!\n");
			break;
		case MIFARE_CADR:
			printf("Mifare card type!\n");
			break;
		case ULTRALIGHT_NFC_CARD:
			printf("Ultralight card type\n");
			break;
		case ISO14443B_CADR:
			printf("ISO14443B card type\n");
			break;
		case ISO14443A_CPU_CARD:
			printf("ISO14443A card type\n");
			break;
		case ISO15693_CARD:
			printf("ISO15693 card type\n");
			break;
		case ID_CARD:
			printf("ID CARD\n");
#ifdef  IDCARD_NETWORK
		//可以通过云上服务器解析协议读取身份证信息  需要与服务器通信 网络编程

#else
		//可以通过本地身份证解析协议读取身份证信息
		ret = idcard_protcol_local(fd,info, sizeof(info), idcard_bitmap, sizeof(idcard_bitmap));
		if(ret < 0)
			return CARDINFO_READ_FAILED;
			
		ret = CARDINFO_READ_OK;
		/*
		printf("文字信息:\n");
		for(int i=0;i<256;i++)
			printf("i=%d 0x%02x ",i,info[i]);
		printf("图片信息:\n");
		for(int i=0;i<1024;i++)
			printf("i=%d 0x%02x ",i,bitmap[i]);
		for(int i=794;i<1024;i++)
			printf("arr[%d] = 0x%02x; ",i,bitmap[i]);
		printf("\n");
		*/
#endif
			break;
		default:
			break;
	}/* end switch */

	return ret;
}


/* 解析卡片信息 analysis card information */
void cardinfo_anylys(int32_t fd,int32_t cardread_status,int32_t card_type)
{
	switch(card_type)                                                                                                      
	{
		case NODEFINE:
			printf("unkown card type!\n");
			break;
		case MIFARE_CADR:
			printf("Mifare card type!\n");
			break;
		case ULTRALIGHT_NFC_CARD:
			printf("Ultralight card type\n");
			break;
		case ISO14443B_CADR:
			printf("ISO14443B card type\n");
			break;
		case ISO14443A_CPU_CARD:
			printf("ISO14443A card type\n");
			break;
		case ISO15693_CARD:
			printf("ISO15693 card type\n");
			break;
		case ID_CARD:
			printf("ID CARD\n");
			idcardinfo_anylys(fd,info,idcard_bitmap);//如果是身份证的话解析身份证并显示
			break;
		default:
			break;

	}//end switch

}
