#include "uart.h"
extern volatile int32_t cardread_status;

/*配置串口参数  波特率  停止位  校验位 */
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) {
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 10;///* 设置超时1 seconds,unit:100ms*/
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}

	return 0;
}

/*初始化串口
 *return:正确返回文件描述符  错误返回负值
*/
int uart_init()
{
	/* 1.打开串口 */
	int fd = open(SERIALNAME,O_RDWR|O_NONBLOCK|O_NDELAY);
	if (fd < 0) {
		printf("串口打开失败\n");
		return UART_RET_OPENFAILED;
	}

	/* 2.配置串口参数 */
	int ret = set_opt(fd, 115200, 8, 'N', 1);
	if(ret < 0) {
		printf("串口配置失败\n");
		return UART_RET_SETFAILED;
	}

	/* 清串口 */
	tcflush(fd,TCIOFLUSH);
	printf("Serial Init Success!!!Baud: 115200, WordLength: 8bit, StopBits:1bit, Parity: none, HardwareFlowControl: none!!!\n");

	return fd;
}

/* 关闭串口 */
int uart_close(int fd)
{
	int ret = close(fd);
	if(ret < 0) {
		perror("close serial");
		return UART_RET_CLOSEFAILED;
	}

	return UART_RET_OK;
}

/* 毫秒级 延时 */
void Delay_ms(int ms)
{
	struct timeval delay;
	delay.tv_sec = 0;
	delay.tv_usec = ms * 1000;
	select(0, NULL, NULL, NULL, &delay);
}

/* 将数据往串口中写：向模块发送指令 */
int uart_send(int fd,uint8_t *buf, uint32_t len)
{
	int ret;
	if(buf ==  NULL || len == 0) {
		return UART_SEND_NULL;
	}

	ret =  write(fd, buf, len);

	if(ret < 0){
		printf("向串口写入数据失败\n");
		return UART_SEND_FAILED;
	}

	printf("write to serial success!!!\n");
	//sleep(1);

	/* debug print */
	for(int i=0;i<len;i++)
		printf("write buf[%d]= 0x%02X  ",i,buf[i]);
	printf("\n");

	return UART_SEND_OK;
}

/* 按字节读取串口数据 */
int uart_read(int fd,uint8_t *buf, int len, uint32_t delay)
{
	/* 读取串口数据 */
	int ret;
	uint8_t rcv_buf;

	for(int j = 0; j < len; j++) {
		do {
			ret = read(fd,&rcv_buf, 1);/* 从串口每次读取一个字节的数据 * len */
			delay--;
		} while( (ret <= 0) && (delay>0) );/* 如果没有读取到则在延迟时间内一直循环 */

		if((delay==0)&&(ret<=0)) {/* 在延迟时间内未读到串口信息 返回错误 */
			return UART_READ_FAILED;
		}

		buf[j] = rcv_buf;
	}
	//sleep(5);
	
	//debug print
	for(int i=0;i<len;i++)
	{
		printf("read buf[%d]= 0x%02X  ",i,buf[i]);
		if(i > 100)
			break;
	}
	printf("\n");

	return UART_READ_OK;;
}


/* 异或和校验 */
int calc_lrc(uint8_t *in_data, int len)
{
	unsigned char lrc;

	if (in_data == NULL || len <= 0) 
		return 0;

	lrc = 0;
	while (len-- > 0) {
		lrc ^= *in_data++;
	}
	return lrc;
}
