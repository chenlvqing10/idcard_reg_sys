#ifndef __IDCARD__
#define __IDCARD__

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include "type.h"
#include "SynReader.h"

#define  IDCARD_ACTIVE_FAILED    -1    /* 身份证激活失败 */
#define  IDCARD_ACTIVE_OK        0     /* 身份证激活成功 */

#define   IDCARD_SELECT_FAILED   -1    /* 身份证选择失败 */
#define   IDCARD_SELECT_OK       0     /* 身份证选择成功 */

#define   IDCARD_READ_FAILED    -1     /* 身份证读取失败 */
#define   IDCARD_READ_OK        0      /* 身份证读取成功 */

#define  MAXSIZE                 1300  /* 身份证数据最大字节数 */
#define  MINSIZE                 0     /* 身份证数据最小字节数 */

/* 协议应答码解析 protocol resp code analysis*/
#define  IDCARD_DATA_ACTIVE_OK    0     /* 激活成功 */
#define  IDCARD_DATA_SELECT_OK    1     /* 选择成功 */
#define  IDCARD_DATA_READ_OK      3     /* 读取成功 */
#define  IDCARD_DATA_NULL        -1     /* 数据为空 */ 
#define  IDCARD_DATA_ERR_WRITE   -2     /* 命令码写入错误 */
#define  IDCARD_DATA_ERR_READ    -3     /* 应答码读取错误 */
#define  IDCARD_DATA_ERR_LRC     -4     /* 异或和校验失败 */
#define  IDCARD_DATA_ERR_LEN     -5     /* 数据长度错误 */
#define  IDCARD_DATA_ERR_CMD     -6     /* 命令 */
#define  IDCARD_DATA_ERR_OTHER   -7     /* 无法识别的其它错误 */
#define  IDCARD_DATA_ERR_SELECT  -8     /* 选择身份证失败 */
#define  IDCARD_DATA_ERR_ACTIVE  -9     /* 激活身份证失败 */

/* 民族字段 national fields*/
const static char nations[57][15] = {" ","汉","蒙古","回","藏","维吾尔","苗","彝","壮","布依","朝鲜","满","侗","瑶","白",
	"土家","哈尼","哈萨克","傣","黎","僳僳","佤","畲","高山","拉祜","水","东乡","纳西","景颇", 
	"柯尔克孜","土","达斡尔","仫佬","羌","布朗","撒拉","毛南","仡佬","锡伯","阿昌","普米","塔吉克","怒", 
	"乌孜别克","俄罗斯","鄂温克","德昂","保安","裕固","京","塔塔尔","独龙","鄂伦春","赫哲","门巴","珞巴","基诺"};

 /* 身份证文字信息结构体  标准 from id card 256 + 1024 bytes Hex data for english and number*/
 typedef struct IDCardData {
     unsigned char name[30];                 /* 姓名          30字节 */
     unsigned char gender[2];                /* 性别          2字节编码  表示数字 实际上汉字是3个字节 */
     unsigned char national[4];              /* 民族          4字节编码  表示数字 最大 = 4*3 = 12个字节 */
     unsigned char birthday[16];             /* 生日          16字节 */
     unsigned char address[70];              /* 地址          70字节 */
     unsigned char idnumber[36];             /* 身份证号      36字节 */                        
     unsigned char maker[30];                /* 签证机关      30字节 */
     unsigned char start_date[16];           /* 有效起始期限  16字节 */
     unsigned char end_date[16];             /* 有效结束期限  16字节 */
     unsigned char reserved[36];             /* 保留字节      36字节 */
 } __attribute__((__packed__)) St_IDCardData, *PSt_IDCardData;

/* 最终的身份证文字信息结构体 中文字节长度需要再构造 reconstruct for chinese */
typedef struct _IDCardData {
    unsigned char name[30];                 /* 姓名          30字节 */
    unsigned char gender[4];                /* 性别          2字节编码  表示数字 实际上汉字是3个字节 */
    unsigned char national[14];             /* 民族          4字节编码  表示数字 最大 = 4*3 = 12个字节 */
    unsigned char birthday[16];				/* 生日          16字节 */
    unsigned char address[70];              /* 地址          70字节 */
    unsigned char idnumber[36];             /* 身份证号      36字节 */                      
    unsigned char maker[50];                /* 签证机关      30字节不够 暂时50字节 */
    unsigned char start_date[16];           /* 有效起始期限  16字节 */
    unsigned char end_date[16];             /* 有效结束期限  16字节 */
    unsigned char reserved[36];             /* 保留字节      36字节 */
} __attribute__((__packed__)) _St_IDCardData, *_PSt_IDCardData;

_St_IDCardData  idcard_info;/* 全局变量 需要将其传递给QT */

/* 函数 fun declare in this */
int32_t getInfoLenth(char* info);      /* 去除空的填充位 0x20 0x00 得到数据的实际长度 用于中文处理 */
int32_t idcard_protcol_local(int32_t fd,uint8_t *info, int32_t info_len, uint8_t *bitmap, int32_t bitmap_len);  /* 通过本地协议解析 */
int32_t idcard_protcol_network(int32_t fd,uint8_t *info, int32_t info_len, uint8_t *bitmap, int32_t bitmap_len); /* 通过云上服务协议解析 fun not coding  */
int32_t idcard_active(int32_t fd);         /* 激活身份证 active idcard */
int32_t idcard_select(int32_t fd);         /* 选择身份证 select idcard */
int32_t idcard_read(int32_t fd,uint8_t *info, int32_t info_len, uint8_t *bitmap, int32_t bitmap_len);	/* 读取身份证信息 read idcard info*/
int32_t idcard_comm_exchange(int32_t fd,uint8_t *cmd, uint8_t *in_data, int32_t in_len, uint8_t *out_data, int32_t out_len);/* 串口交互函数 judge the resp code is true*/
void idcardinfo_anylys(int32_t fd,uint8_t *info, uint8_t *bitmap);/* 解析身份证 analysis idcard*/
void getName(St_IDCardData info,char* Name); /* 得到身份证姓名 */
void getGender(St_IDCardData info,char* Gender); /* 得到身份证性别 */
void getNational(St_IDCardData info,char*National); /* 得到身份证民族 */
void getBirthday(St_IDCardData info,char*Birthday); /* 得到身份证生日 */
void getAddress(St_IDCardData info,char*Address); /* 得到身份证住址 */
void getIDCardNumber(St_IDCardData info,char*IDCardNumber); /* 得到身份证号码 */
void getMaker(St_IDCardData info,char*Maker); /* 得到身份证签发机关 */
void getStartDate(St_IDCardData info,char*StartDate); /* 得到身份证有效起始日期 */
void getEndDate(St_IDCardData info,char*EndDate); /* 得到身份证有效结束日期 */

#endif
