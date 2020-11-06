#ifndef __CARD__
#define __CARD__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "type.h"
#include "main.h"

//#define  IDCARD_NETWORK     /* 云上身份证解析协议  通过网络 */ 
#define  IDCARD_LOCAL         /* 本地身份证解析协议 */

/* 卡片类型的定义 define card type */
#define NODETECT               0xFF   /* 未检出卡片 no detecing card */
#define NODEFINE		       0x00   /* 未定义卡片 no define   card */
#define MIFARE_CADR		       0x01   /* M1卡片 */
#define ULTRALIGHT_NFC_CARD    0x02   /* NFC卡片 */
#define ISO14443B_CADR         0x03   /* ISO14443B卡片  身份证属于此类卡片 when reading card type using "AA 01 02",if is id card,return 0x03 */
#define ISO14443A_CPU_CARD     0x04   /* ISO14443A CPU卡片 */
#define ISO15693_CARD          0x05   /* ISO15693卡片 */
#define ID_CARD                0x06   /* 从帧头BB识别的身份证卡片 */

#define CARDINFO_READ_FAILED   -1     /* 读卡失败 reading card failed */
#define CARDINFO_READ_OK       0	  /* 读卡成功 reading card success */

/* 串口通信指令数据帧 serial communication code date frame */
#define FRAME_HEAD              "\xAA"   /* 帧头 */
#define FRAME_HEAD_LEN          1        /* 帧头的长度 */
#define FRAME_CMD_LEN           1        /* 命令码 */
#define FRAME_LENS              1        /* 长度 */
#define FRAME_ADDR_LEN          1        /* 地址 */
#define FRAME_DATA                       /* 数据 */

/* 串口通信指令错误反馈 serial communication code error resp */
#define ERR_TAG_TYPE            "\xAA\x01\xE0"   /* 卡片类型错误 */ 
#define ERR_NO_FIND_TAG         "\xAA\x01\xE1"   /* 未寻到卡错误 */
#define ERR_KEY_NO_AUTH         "\xAA\x01\xE2"   /* M1卡密钥不匹配错误 */
#define ERR_READ_BLOCK          "\xAA\x01\xE3"   /* 读块错误 */
#define ERR_WRITE_BLOCK         "\xAA\x01\xE4"   /* 写块错误 */
#define ERR_VALUE_INIT          "\xAA\x01\xE5"   /* M1卡电子钱包充值初始化错误 */
#define ERR_VALUE_ADD           "\xAA\x01\xE6"   /* M1卡电子钱包充值错误 */
#define ERR_VALUE_SUB           "\xAA\x01\xE7"   /* M1卡电子钱包扣款错误 */
#define CARD_MOVEDD             "\xAA\x01\xEA"   /* 卡片移开 */


/* 身份证云解析通讯协议  需要相应的模块  需要联网   程序代码未实现  idcard cloud analysis communication protocol,fun no coding */
#define FRAME_OL_HEAD              "\xBB"         /* 帧头 */
#define FRAME_OL_HEAD_LEN           1             /* 帧头 len */
#define FRAME_OL_CMD_LEN            1			  /* 命令码 len */
#define FRAME_OL_LENS               2             /* 命令长度 + 地址长度  2 byte */
#define FRAME_OL_ADDR_LEN           1             /* 地址  len */
#define FRAME_OL_DATA                             /* 数据 */
#define FRAME_OL_LRC_LEN            1             /* 校验 len */

/* 本地身份证解析通讯协议 local idcard analysis communication protocol ;fun coding*/
#define FRAME_OF_HEAD               "\xAA\xAA\xAA\x96\x69"  /* 帧头 */
#define FRAME_OF_HEAD_LEN           5						/*  帧头 len */
#define FRAME_OF_CMD_LEN            2						/* 命令码长度 = 命令码 len + 参数 len */
#define FRAME_OF_LENS               2						/* 长度len */
#define FRAME_OF_ADDR_LEN           1						/* 地址 len */
#define FRAME_OF_RESP_LEN           3						/* 应答码 len */
#define FRAME_OF_DATA										/* 数据帧 len */
#define FRAME_OF_LRC_LEN            1						/* 校验码 len */

/* 通用操作 General operation code*/
#define CMD_GET_CARD_UID        0x01     /* 获取卡片UID */
#define CMD_GET_CARD_TYPE       0x02     /* 获取卡片类型 */
#define CMD_SET_DETECT          0x95     /* 自动寻卡开关 */
#define CMD_SET_BAUD            0xA0     /* 修改模块波特率 */
#define CMD_SET_SYS_PARA        0xA1     /* 配置系统参数指令 */
#define CMD_GET_SYS_PARA        0xA2     /* 读取系统参数指令 */
#define CMD_GET_SW_VER          0xB0     /* 获取模块固件版本号 */
#define CMD_GET_HW_VER          0xB1     /* 获取模块硬件版本号 */

/* MIFARE OPS */
#define CMD_M1_WRITE_A_KEY      0x03     /* 向模块写入需要验证的密钥(A密钥) */
#define CMD_M1_READ_BLK         0x04	 /* Mifare卡读块 */
#define CMD_M1_WRITE_BLK        0x05	 /* Mifare卡写块 */
#define CMD_M1_OPER_INIT        0x06	 /* Mifare卡增减值初始化 */
#define CMD_M1_OPER_ADD         0x07	 /* Mifare卡增值 */
#define CMD_M1_OPER_MINUS       0x08	 /* Mifare卡减值 */
#define CMD_M1_WRITE_B_KEY      0x0B	 /* 向模块写入需要验证的密钥(B密钥) */
#define CMD_M1_SET_KEY_TYPE     0x0C     /* 设置模块使用密钥的类型 */

/* ULTRALIGHT OPS */
#define CMD_UL_READ_BLK         0x09	 /* Ultralight卡读块 */
#define CMD_UL_READ_MBLK        0x1C	 /* Ultralight卡读多个 */
#define CMD_UL_WRITE_MBLK       0x1D	 /* Ultralight卡写多个 */
/* ISO14443 CPU CARD OPS */
#define CMD_ICC_ACTIVE          0x15	 /* ISO14443-A CPU卡片激活指令 */
#define CMD_ICC_APDU_EXCHANGE   0x17     /* ISO14443-A CPU卡APDU指令接口 */
#define CMD_ICC_DEACTIVE        0x18 	 /* 卡片断电指令、关闭天线指令接口 */

/* ID CARD OPS */
#define CMD_IDC_ACTIVE          0x14	 /* 身份证激活指令 */
#define CMD_IDC_APDU_EXCHANGE   0x16	 /* 身份证APDU指令接口 */
#define CMD_IDC_DEACTIVE        0x18	 /* 卡片断电指令、关闭天线指令接口 */

/* ISO15693 CARD OPS */
#define CMD_ISO_READ_BLK        0x90	 /* ISO15693卡读单个块 */
#define CMD_ISO_READ_MBLK       0x91	 /* ISO15693读多个块 */
#define CMD_ISO_WRITE_BLK       0x92	 /* ISO15693写单个块 */
#define CMD_ISO_WIRTE_MBLK      0x93	 /* ISO15693写多个块 */
#define CMD_ISO_LOCK_CLK        0x94	 /* ISO15693锁住块 */

/* ERROR RESPONSE */
#define CMD_ERR_CARD_TYPE       0xE0	 /* 卡类型错误反馈指令 */
#define CMD_ERR_NOT_DETECT      0xE1	 /* 未寻到卡错误反馈指令 */
#define CMD_ERR_UNMATCH_KEY     0xE2	 /* 密钥不匹配错误反馈指令 */
#define CMD_ERR_READ_BLK        0xE3	 /* 读块失败错误指令 */
#define CMD_ERR_WRITE_BLK       0xE4	 /* 写块失败错误指令 */
#define CMD_ERR_M1_INIT         0xE5	 /* M1卡值初始化失败错误指令 */
#define CMD_ERR_M1_ADD          0xE6	 /* M1卡增值失败错误指令 */
#define CMD_ERR_M1_MINUS        0xE7	 /* M1卡减值失败错误指令 */
#define CMD_ACK                 0xFE	 /* ACK确认命令 */
#define CMD_NACK                0xFF	 /* NACK否认命令 */

/* 命令码 command code */
#define CMD_IDC_ACTIVEED        "\x20\x01"	/* 身份证激活 idcard active */
#define CMD_IDC_SELECT          "\x20\x02"  /* 身份证选择 idcard select */
#define CMD_IDC_READ_PLAIN      "\x30\x01"  /* 身份证读取 idcard reading */

/* 应答码 replay code */
#define CMD_IDC_RESP_EXEC_OK        "\x00\x00\x90"		/* 操作成功 */
#define CMD_IDC_RESP_DETECT_OK      "\x00\x00\x9F"		/* 寻找身份证成功 */
#define CMD_IDC_RESP_ERR_LRC        "\x00\x00\x10"		/* 接收上位机数据的校验和错 */
#define CMD_IDC_RESP_ERR_LEN        "\x00\x00\x11"		/* 接收上位机数据的长度错 */
#define CMD_IDC_RESP_ERR_CMD        "\x00\x00\x21"		/* 接收上位机命令错，包括命令钟的各种数值或逻辑搭配错误 */
#define CMD_IDC_RESP_ERR_OTHERS     "\x00\x00\x24"		/* 无法识别的错误 */
#define CMD_IDC_RESP_ERR_READ       "\x00\x00\x41"		/* 读身份证操作失败 */
#define CMD_IDC_RESP_ERR_DETECT     "\x00\x00\x80"		/* 寻找身份证失败 */
#define CMD_IDC_RESP_ERR_SELECT     "\x00\x00\x81"		/* 选择身份证失败 */

/* 函数 fun declare in this */
int32_t  card_detect(int32_t fd,int32_t cardread_status);						/* 检测不同类型的卡片  detect card type */
bool is_cardMoved(int32_t fd);													/* 检测卡片是否被移除  detect card is moved */
int32_t  cardinfo_read(int32_t fd,int32_t cardread_status,int32_t card_type);	/* 读取卡片信息        reading card info */
void cardinfo_anylys(int32_t fd,int32_t cardread_status,int32_t card_type);		/* 解析卡片信息        analysis card info */

#endif 

