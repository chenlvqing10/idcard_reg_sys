//进行C代码动态链接库的封装  以供QT应用程序使用的接口
#ifndef __IDCARDREGNTON__
#define __IDCARDREGNTON__

//头文件
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <strings.h>


void print_hello();




//define 定义
typedef signed char             int8_t;   
typedef unsigned char           uint8_t;  
typedef int                     int32_t;  
typedef unsigned int            uint32_t;  
typedef unsigned char           uint8_t;  
typedef float                   float32_t;
typedef double                  float64_t;
//卡片状态
enum CARDREAD_STATUS 
{
    CARDREAD_STATUS_NO  = 0,//未读卡
    CARDREAD_STATUS_FAILED,//读卡失败
    CARDREAD_STATUS_ING,//读卡中
    CARDREAD_STATUS_MOVED,//卡片移开
    CARDREAD_STATUS_OK//读卡结束
};  
//卡片类型的定义
#define NODETECT               0xFF
#define NODEFINE               0x00
#define MIFARE_CADR            0x01
#define ULTRALIGHT_NFC_CARD    0x02
#define ISO14443B_CADR         0x03
#define ISO14443A_CPU_CARD     0x04
#define ISO15693_CARD          0x05
#define ID_CARD                0x06

#define CARDINFO_READ_FAILED   -1
#define CARDINFO_READ_OK       0

//卡片指令/数据帧格式
//串口通信指令数据帧
#define FRAME_HEAD              "\xAA"   //帧头
#define FRAME_HEAD_LEN          1
#define FRAME_CMD_LEN           1
#define FRAME_LENS              1
#define FRAME_ADDR_LEN          1
#define FRAME_DATA

//串口错误反馈
#define ERR_TAG_TYPE            "\xAA\x01\xE0"   //卡片类型错误 
#define ERR_NO_FIND_TAG         "\xAA\x01\xE1"   //未寻到卡错误
#define ERR_KEY_NO_AUTH         "\xAA\x01\xE2"   //M1卡密钥不匹配错误
#define ERR_READ_BLOCK          "\xAA\x01\xE3"   //读块错误
#define ERR_WRITE_BLOCK         "\xAA\x01\xE4"   //写块错误
#define ERR_VALUE_INIT          "\xAA\x01\xE5"   //M1卡电子钱包充值初始化错误
#define ERR_VALUE_ADD           "\xAA\x01\xE6"   //M1卡电子钱包充值错误
#define ERR_VALUE_SUB           "\xAA\x01\xE7"   //M1卡电子钱包扣款错误
#define CARD_MOVEDD             "\xAA\x01\xEA"   //卡片移开
 /*身份证云解析通讯协议  需要相应的模块  需要联网   程序代码需扩展  网络编程  服务器?*/
 #define FRAME_OL_HEAD              "\xBB"         //帧头
 #define FRAME_OL_HEAD_LEN           1             //帧头 len
 #define FRAME_OL_CMD_LEN            1             //命令码 len
 #define FRAME_OL_LENS               2             //命令长度 + 地址长度  2 byte
 #define FRAME_OL_ADDR_LEN           1             //地址  len
 #define FRAME_OL_DATA                             //数据
 #define FRAME_OL_LRC_LEN            1             //校验 len
 
 /*本地身份证解析通讯协议*/
 #define FRAME_OF_HEAD               "\xAA\xAA\xAA\x96\x69"
 #define FRAME_OF_HEAD_LEN           5   //帧头 len
 #define FRAME_OF_CMD_LEN            2   //命令码长度 = 命令码 len + 参数 len
 #define FRAME_OF_LENS               2   //长度len
 #define FRAME_OF_ADDR_LEN           1   //地址 len
 #define FRAME_OF_RESP_LEN           3   //应答码 len
 #define FRAME_OF_DATA
 #define FRAME_OF_LRC_LEN            1   //校验码 len
 
 /* 通用操作 */
 #define CMD_GET_CARD_UID        0x01     //获取卡片UID
 #define CMD_GET_CARD_TYPE       0x02     //获取卡片类型
 #define CMD_SET_DETECT          0x95     //自动寻卡开关
 #define CMD_SET_BAUD            0xA0     //修改模块波特率
 #define CMD_SET_SYS_PARA        0xA1     //配置系统参数指令
 #define CMD_GET_SYS_PARA        0xA2     //读取系统参数指令
 #define CMD_GET_SW_VER          0xB0     //获取模块固件版本号
 #define CMD_GET_HW_VER          0xB1     //获取模块硬件版本号
 /* MIFARE OPS */
 #define CMD_M1_WRITE_A_KEY      0x03     //向模块写入需要验证的密钥(A密钥)
 #define CMD_M1_READ_BLK         0x04     //Mifare卡读块
 #define CMD_M1_WRITE_BLK        0x05     //Mifare卡写块
 #define CMD_M1_OPER_INIT        0x06     //Mifare卡增减值初始化
 #define CMD_M1_OPER_ADD         0x07     //Mifare卡增值
 #define CMD_M1_OPER_MINUS       0x08     //Mifare卡减值
 #define CMD_M1_WRITE_B_KEY      0x0B     //向模块写入需要验证的密钥(B密钥)
 #define CMD_M1_SET_KEY_TYPE     0x0C     //设置模块使用密钥的类型
 
 /* ULTRALIGHT OPS */
 #define CMD_UL_READ_BLK         0x09     //Ultralight卡读块
 #define CMD_UL_READ_MBLK        0x1C     //Ultralight卡读多个
 #define CMD_UL_WRITE_MBLK       0x1D     //Ultralight卡写多个
 /* ISO14443 CPU CARD OPS */
 #define CMD_ICC_ACTIVE          0x15     //ISO14443-A CPU卡片激活指令
 #define CMD_ICC_APDU_EXCHANGE   0x17     //ISO14443-A CPU卡APDU指令接口     
 #define CMD_ICC_DEACTIVE        0x18     //卡片断电指令、关闭天线指令接口
 
 /* ID CARD OPS */
 #define CMD_IDC_ACTIVE          0x14     //身份证激活指令
 #define CMD_IDC_APDU_EXCHANGE   0x16     //身份证APDU指令接口
 #define CMD_IDC_DEACTIVE        0x18     //卡片断电指令、关闭天线指令接口
 
 /* ISO15693 CARD OPS */
 #define CMD_ISO_READ_BLK        0x90     //ISO15693卡读单个块
 #define CMD_ISO_READ_MBLK       0x91     //ISO15693读多个块
 #define CMD_ISO_WRITE_BLK       0x92     //ISO15693写单个块
 #define CMD_ISO_WIRTE_MBLK      0x93     //ISO15693写多个块
 #define CMD_ISO_LOCK_CLK        0x94     //ISO15693锁住块
/* ERROR RESPONSE */
#define CMD_ERR_CARD_TYPE       0xE0     //卡类型错误反馈指令
#define CMD_ERR_NOT_DETECT      0xE1     //未寻到卡错误反馈指令
#define CMD_ERR_UNMATCH_KEY     0xE2     //密钥不匹配错误反馈指令
#define CMD_ERR_READ_BLK        0xE3     //读块失败错误指令
#define CMD_ERR_WRITE_BLK       0xE4     //写块失败错误指令
#define CMD_ERR_M1_INIT         0xE5     //M1卡值初始化失败错误指令
#define CMD_ERR_M1_ADD          0xE6     //M1卡增值失败错误指令
#define CMD_ERR_M1_MINUS        0xE7     //M1卡减值失败错误指令
#define CMD_ACK                 0xFE     //ACK确认命令
#define CMD_NACK                0xFF     //NACK否认命令

//命令码
#define CMD_IDC_ACTIVEED        "\x20\x01"
#define CMD_IDC_SELECT          "\x20\x02"
#define CMD_IDC_READ_PLAIN      "\x30\x01"

//应答码
#define CMD_IDC_RESP_EXEC_OK        "\x00\x00\x90"
#define CMD_IDC_RESP_DETECT_OK      "\x00\x00\x9F"
#define CMD_IDC_RESP_ERR_LRC        "\x00\x00\x10"
#define CMD_IDC_RESP_ERR_LEN        "\x00\x00\x11"
#define CMD_IDC_RESP_ERR_CMD        "\x00\x00\x21"
#define CMD_IDC_RESP_ERR_OTHERS     "\x00\x00\x24"
#define CMD_IDC_RESP_ERR_READ       "\x00\x00\x41"
#define CMD_IDC_RESP_ERR_DETECT     "\x00\x00\x80"
#define CMD_IDC_RESP_ERR_SELECT     "\x00\x00\x81"

#define SERIALNAME    "/dev/ttyS0"        //串口设备名

#define UART_RET_OK              0        //串口OK
#define UART_RET_OPENFAILED      -1       //串口打开失败
#define UART_RET_SETFAILED       -2       //串口设置失败
#define UART_RET_CLOSEFAILED     -3       //串口关闭失败

#define UART_SEND_OK             0        //串口发送数据成功
#define UART_SEND_NULL           -4       //串口发送空数据
#define UART_SEND_FAILED         -5       //串口发送数据失败

#define UART_READ_OK            0         //读取串口数据成功
#define UART_READ_FAILED       -6         //读取串口数据失败

typedef unsigned char   boolean;
typedef unsigned int    CharType ;
typedef unsigned char   UTF8;
typedef unsigned short  UTF16;
typedef unsigned int    UTF32;

#define FALSE  0
#define TRUE   1

#define halfShift   10
#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF
/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF
static const UTF32 halfMask = 0x3FFUL;
static const UTF32 halfBase = 0x0010000UL;
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };
static const char trailingBytesForUTF8[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
typedef enum
{
    strictConversion = 0,
    lenientConversion
} ConversionFlags;
typedef enum
{
    conversionOK,       /* conversion successful */
    sourceExhausted,    /* partial character in source, but hit end */
    targetExhausted,    /* insuff. room in target for conversion */
    sourceIllegal,      /* source sequence is illegal/malformed */
    conversionFailed
} ConversionResult;






#define  IDCARD_ACTIVE_FAILED    -1
#define  IDCARD_ACTIVE_OK        0

#define   IDCARD_SELECT_FAILED   -1
#define   IDCARD_SELECT_OK       0

#define   IDCARD_READ_FAILED    -1
#define   IDCARD_READ_OK        0

#define  MAXSIZE                 1300
#define  MINSIZE                 0
#define  IDCARD_DATA_ACTIVE_OK    0     //激活成功
#define  IDCARD_DATA_SELECT_OK    1     //选择成功
#define  IDCARD_DATA_READ_OK      3     //读取成功
#define  IDCARD_DATA_NULL        -1   //数据为空 
#define  IDCARD_DATA_ERR_WRITE   -2   //命令码写入错误
#define  IDCARD_DATA_ERR_READ    -3   //应答码读取错误
#define  IDCARD_DATA_ERR_LRC     -4   //异或和校验失败
#define  IDCARD_DATA_ERR_LEN     -5   //数据长度错误
#define  IDCARD_DATA_ERR_CMD     -6   //命令
#define  IDCARD_DATA_ERR_OTHER   -7   //无法识别的其它错误
#define  IDCARD_DATA_ERR_SELECT  -8   //选择身份证失败
#define  IDCARD_DATA_ERR_ACTIVE  -9   //激活身份证失败
const static char nations[57][15] = {" ","汉","蒙古","回","藏","维吾尔","苗","彝","壮","布依","朝鲜","满","侗","瑶","白",
    "土家","哈尼","哈萨克","傣","黎","僳僳","佤","畲","高山","拉祜","水","东乡","纳西","景颇", 
    "柯尔克孜","土","达斡尔","仫佬","羌","布朗","撒拉","毛南","仡佬","锡伯","阿昌","普米","塔吉克","怒", 
    "乌孜别克","俄罗斯","鄂温克","德昂","保安","裕固","京","塔塔尔","独龙","鄂伦春","赫哲","门巴","珞巴","基诺"};

 //身份证文字信息结构体
 typedef struct IDCardData {
     unsigned char name[30];                 //姓名          30字节
     unsigned char gender[2];                //性别          2字节
     unsigned char national[4];              //民族          4字节
     unsigned char birthday[16];             //生日          16字节
     unsigned char address[70];              //地址          70字节
     unsigned char idnumber[36];                 //身份证号      36字节                         
     unsigned char maker[30];                //签证机关      30字节
     unsigned char start_date[16];           //有效起始期限  16字节
     unsigned char end_date[16];             //有效结束期限  16字节
     unsigned char reserved[36];             //保留字节      36字节
 } __attribute__((__packed__)) St_IDCardData, *PSt_IDCardData;
 
//函数
//主接口函数
#ifdef __cplusplus
extern "C"
{
#endif

ConversionResult Utf8_To_Utf16 (const UTF8* sourceStart, UTF16* targetStart, size_t outLen , ConversionFlags flags) ;
int Utf16_To_Utf8 (const UTF16* sourceStart, UTF8* targetStart, size_t outLen ,  ConversionFlags flags)  ;
int idcardregnton(uint8_t** ptr_idcard_info,uint8_t** ptr_idcard_bitmap,St_IDCardData** ptr_st_idcard_info);
//身份证识别流程函数
int  card_detect(int fd,int cardread_status);               //检测不同类型的卡片
bool is_cardMoved(int fd);              //检测卡片是否被移除
int  cardinfo_read(int fd,int cardread_status,int card_type);    //读取卡片信息
void cardinfo_anylys(int fd,int cardread_status,int card_type);    //解析卡片信息

//串口操作函数
int uart_init(void);//初始化串口                                                                  
int uart_close(int fd);//关闭串口
int uart_read(int fd,uint8_t *buf, int len, uint32_t delay);//读取串口数据    接收
int uart_send(int fd,uint8_t *buf, uint32_t len);//写入数据到串口  发送
int uart_setopt(int fd);//配置串口参数 
void Delay_ms(int ms);//毫秒级延迟函数
int calc_lrc(uint8_t *in_data, int len);

//身份证读取解析函数
int getInfoLenth(char* info);      //去除空的填充为 0x20 0x00 得到数据的实际长度 用于中文处理
int idcard_protcol_local(int fd,uint8_t *info, int info_len, uint8_t *bitmap, int bitmap_len);  //通过本地协议解析
int idcard_protcol_network(int fd,uint8_t *info, int info_len, uint8_t *bitmap, int bitmap_len);//通过云上服务协议解析 未实现
int idcard_active(int fd);         //激活身份证
int idcard_select(int fd);         //选择身份证
int idcard_read(int fd,uint8_t *info, int info_len, uint8_t *bitmap, int bitmap_len);//读取身份证信息
int idcard_comm_exchange(int fd,uint8_t *cmd, uint8_t *in_data, int in_len, uint8_t *out_data, int out_len);//串口交互函数  实现数据的收发
void idcardinfo_anylys(int fd,uint8_t *info, uint8_t *bitmap);
void getName(St_IDCardData info,char* Name);
void getGender(St_IDCardData info,char* Gender);
void getNational(St_IDCardData info,char*National);
void getBirthday(St_IDCardData info,char*Birthday);
void getAddress(St_IDCardData info,char*Address);
void getIDCardNumber(St_IDCardData info,char*IDCardNumber);
void getMaker(St_IDCardData info,char*Maker);
void getStartDate(St_IDCardData info,char*StartDate);
void getEndDate(St_IDCardData info,char*EndDate);
#ifdef __cplusplus
};
#endif

#endif
