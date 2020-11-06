#ifndef __UART__
#define __UART__

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <strings.h>
#include "type.h"
#define SERIALNAME    "/dev/ttyS0"        /* 串口设备名 */

#define UART_RET_OK              0        /* 串口OK */
#define UART_RET_OPENFAILED      -1       /* 串口打开失败 */
#define UART_RET_SETFAILED       -2       /* 串口设置失败 */
#define UART_RET_CLOSEFAILED     -3       /* 串口关闭失败 */

#define UART_SEND_OK             0        /* 串口发送数据成功 */
#define UART_SEND_NULL           -4       /* 串口发送空数据 */
#define UART_SEND_FAILED         -5       /* 串口发送数据失败 */

#define UART_READ_OK            0	      /* 读取串口数据成功 */
#define UART_READ_FAILED       -6         /* 读取串口数据失败 */

 /* 函数 fun declare in this */
int uart_init(void);/* 初始化串口 */
int uart_close(int fd);/* 关闭串口 */
int uart_read(int fd,uint8_t *buf, int len, uint32_t delay);/* 读取串口数据    接收 */
int uart_send(int fd,uint8_t *buf, uint32_t len);/* 写入数据到串口  发送 */
int uart_setopt(int fd);/* 配置串口参数 */
void Delay_ms(int ms);/* 毫秒级延迟函数 */
int calc_lrc(uint8_t *in_data, int len); /* 异或和校验 */


#endif
