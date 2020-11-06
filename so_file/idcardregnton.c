//动态链接库函数具体的实现
#include "idcardregnton.h"

volatile static int32_t cardread_status = CARDREAD_STATUS_NO;//读卡的状态 全局变量 整个程序可见
volatile static uint8_t cardtype = NODETECT;
static uint8_t info[256] = {0};
static uint8_t bitmap[1024] = {0};
static St_IDCardData idcard_info = {0};

void print_hello()
{
	printf("hello world\n");
}


uint8_t get_frame_head(int fd)
{
	//读取串口的第一个字节(帧头)
	uint8_t  frame_head = '\0';
	int ret = uart_read(fd,&frame_head,1,1000);
	if(ret < 0)
	{
		printf("帧头读取失败\n");
		frame_head = '\xFF';
	}
	else
	{
		printf("模块返回的帧头是:0x%2X\n",frame_head);
	}

	return frame_head;
}

uint8_t get_cardtype(int fd)
{
	//读取串口返回的卡片类型
	uint8_t* frameinfo = NULL;
	int ret = uart_read(fd,frameinfo,4,1000);
	if(ret < 0)
	{
		printf("串口读取失败\n");
		cardtype = NODETECT;
	}
	else
	{
		cardtype = frameinfo[3];
		printf("模块返回的卡片类型是:0x%2X",cardtype);
	}

	return cardtype;
}

//检测不同类型的卡片
int  card_detect(int fd,int cardread_status)
{
	//如果前期已经检测到了卡片,则直接返回卡片类型
	if(cardread_status != CARDREAD_STATUS_NO)
		return cardtype;

	//1.读取模块通过串口返回的帧头
	uint8_t frame_head_res = get_frame_head(fd);
	if(frame_head_res == NODETECT)
		return cardtype;
	else
	{
		if(!memcmp(&frame_head_res,FRAME_HEAD,1))//"AA"  非身份证卡片
		{
			cardtype = get_cardtype(fd);
		}
		else if(!memcmp(&frame_head_res,FRAME_OL_HEAD,1))//"BB" 身份证卡片
		{
			cardtype = ID_CARD;
		}
		else//乱码  可能硬件问题  接线电平问题
		{
			cardtype = NODETECT;
		}
	}

	return cardtype;
}

bool is_cardMoved(int fd)
{
	printf("call %s line = %d\n",__FUNCTION__,__LINE__);
	uint8_t buf[3];
	int ret = read(fd,buf,3);

	for(int i=0;i<3;i++)
		printf("moved buf[%d] = 0x%02x",i,buf[i]);

	if(ret < 0)
	{
		printf("card moved  串口读取失败\n");
		return false;
	}

	if(!memcmp(&buf[0],"\x00",1))//输入串口无数据
	{
		printf("card moved 输入串口无数据\n");
		return false;
	}

	if(!memcmp(&buf[1],"\xAA",1))
	{
		printf("card moved 非卡片移动帧\n");
		return false;
	}
	sleep(5);

	if(!memcmp(buf,CARD_MOVEDD,3))
		ret = true;

	printf("卡片移除判断函数结束\n");
	return ret;
}


//读取卡片信息
int  cardinfo_read(int fd,int cardread_status,int card_type)
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

			//检查卡片是否快速移开
			bool is_moved = is_cardMoved(fd);
			if(is_moved == true)
			{
				printf("卡片被移除了\n");
				cardread_status = CARDREAD_STATUS_MOVED;
				return CARDINFO_READ_FAILED;
			}
			else
			{
				printf("卡片未被移除\n");
			}

			Delay_ms(5000);

			//可以通过云上服务器解析协议读取身份证信息  需要与服务器通信 网络编程

			//可以通过本地身份证解析协议读取身份证信息
			ret = idcard_protcol_local(fd,info, sizeof(info), bitmap, sizeof(bitmap));
			if(ret < 0)
				return CARDINFO_READ_FAILED;

			ret = CARDINFO_READ_OK;

			printf("文字信息:\n");
			for(int i=0;i<256;i++)
				printf("i=%d 0x%02x ",i,info[i]);
			printf("图片信息:\n");
			for(int i=0;i<1024;i++)
				printf("i=%d 0x%02x ",i,bitmap[i]);
			printf("\n");
			break;
		default:
			break;
	}//end switch

	return ret;
}


void cardinfo_anylys(int fd,int cardread_status,int card_type)    //解析卡片信息
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
			idcardinfo_anylys(fd,info,bitmap);//如果是身份证的话解析身份证并显示
			break;
		default:
			break;

	}//end switch

}

//配置串口参数  波特率  停止位  校验位
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

//初始化串口
//return:正确返回文件描述符  错误返回负值
int uart_init()
{
	//1.打开串口
	int fd = open(SERIALNAME,O_RDWR|O_NONBLOCK|O_NDELAY);
	if (fd < 0)
	{
		printf("串口打开失败\n");
		return UART_RET_OPENFAILED;
	}

	//2.配置串口参数
	int ret = set_opt(fd, 115200, 8, 'N', 1);
	if(ret < 0)
	{
		printf("串口配置失败\n");
		return UART_RET_SETFAILED;
	}

	//清串口
	tcflush(fd,TCIOFLUSH);
	printf("Serial Init Success!!!Baud: 115200, WordLength: 8bit, StopBits:1bit, Parity: none, HardwareFlowControl: none!!!\n");

	return fd;
}

//关闭串口
int uart_close(int fd)
{
	int ret = close(fd);
	if(ret < 0)
	{
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

//将数据往串口中写：向模块发送指令
int uart_send(int fd,uint8_t *buf, uint32_t len)
{
	int ret;
	if(buf ==  NULL || len == 0)
	{
		return UART_SEND_NULL;
	}

	ret =  write(fd, buf, len);

	if(ret < 0)
	{
		printf("向串口写入数据失败\n");
		return UART_SEND_FAILED;
	}

	printf("write to serial success!!!\n");
	//sleep(1);

	for(int i=0;i<len;i++)
		printf("write buf[%d]= 0x%02X  ",i,buf[i]);
	printf("\n");

	return UART_SEND_OK;
}

//按字节读取串口数据
int uart_read(int fd,uint8_t *buf, int len, uint32_t delay)
{
	/*
	//当每次读取串口数据的时候 唤醒卡片移除判断线程
	if(cardread_status == CARDREAD_STATUS_ING)  //读卡中 且  线程未结束
	{
	pthread_mutex_lock(&mutex);
	//唤醒线程
	printf("唤醒线程\n");
	pthread_cond_signal(&cond);

	//如果线程使得卡片读取状态变为NO
	if(cardread_status == CARDREAD_STATUS_MOVED)
	{
	return UART_READ_FAILED;
	}
	pthread_mutex_unlock(&mutex);
	//阻塞子线程  等待子线程结束
	pthread_join(cardmoved_pid,NULL);
	}

	printf("不处理卡片唤醒线程\n");
	*/
	//读取串口数据
	int ret;
	uint8_t rcv_buf;

	for(int j = 0; j < len; j++)
	{
		do
		{
			ret = read(fd,&rcv_buf, 1);//从串口每次读取一个字节的数据 * len
			delay--;
		}
		while( (ret <= 0) && (delay>0) );//如果没有读取到则在延迟时间内一直循环

		if((delay==0)&&(ret<=0))//在延迟时间内未读到串口信息 返回错误
		{
			return UART_READ_FAILED;
		}

		buf[j] = rcv_buf;
	}
	sleep(5);

	for(int i=0;i<len;i++)
		printf("read buf[%d]= 0x%02X  ",i,buf[i]);
	printf("\n");

	return UART_READ_OK;;
}


//异或和校验
int calc_lrc(uint8_t *in_data, int len)
{
	unsigned char lrc;

	if (in_data == NULL || len <= 0) return 0;

	lrc = 0;
	while (len-- > 0) {
		lrc ^= *in_data++;
	}
	return lrc;
}
static boolean isLegalUTF8(const UTF8 *source, int length)
{
	UTF8 a;
	const UTF8 *srcptr = NULL;

	if (NULL == source){
		printf("ERR, isLegalUTF8: source=%p\n", source);
		return FALSE;
	}
	srcptr = source+length;

	switch (length) {
		default:
			printf("ERR, isLegalUTF8 1: length=%d\n", length);
			return FALSE;
			/* Everything else falls through when "TRUE"... */
		case 4:
			if ((a = (*--srcptr)) < 0x80 || a > 0xBF){
				printf("ERR, isLegalUTF8 2: length=%d, a=%x\n", length, a);
				return FALSE;
			}
		case 3:
			if ((a = (*--srcptr)) < 0x80 || a > 0xBF){
				printf("ERR, isLegalUTF8 3: length=%d, a=%x\n", length, a);
				return FALSE;
			}
		case 2:
			if ((a = (*--srcptr)) > 0xBF){
				printf("ERR, isLegalUTF8 4: length=%d, a=%x\n", length, a);
				return FALSE;
			}
			switch (*source)
			{
				/* no fall-through in this inner switch */
				case 0xE0:
					if (a < 0xA0){
						printf("ERR, isLegalUTF8 1: source=%x, a=%x\n", *source, a);
						return FALSE;
					}
					break;
				case 0xED:
					if (a > 0x9F){
						printf("ERR, isLegalUTF8 2: source=%x, a=%x\n", *source, a);
						return FALSE;
					}
					break;
				case 0xF0:
					if (a < 0x90){
						printf("ERR, isLegalUTF8 3: source=%x, a=%x\n", *source, a);
						return FALSE;
					}
					break;
				case 0xF4:
					if (a > 0x8F){
						printf("ERR, isLegalUTF8 4: source=%x, a=%x\n", *source, a);
						return FALSE;
					}
					break;
				default:
					if (a < 0x80){
						printf("ERR, isLegalUTF8 5: source=%x, a=%x\n", *source, a);
						return FALSE;
					}
			}
		case 1:
			if (*source >= 0x80 && *source < 0xC2){
				printf("ERR, isLegalUTF8: source=%x\n", *source);
				return FALSE;
			}
	}
	if (*source > 0xF4)
		return FALSE;
	return TRUE;
}
ConversionResult Utf8_To_Utf16 (const UTF8* sourceStart, UTF16* targetStart, size_t outLen , ConversionFlags flags)
{
	ConversionResult result = conversionOK;
	const UTF8* source = sourceStart;
	UTF16* target      = targetStart;
	UTF16* targetEnd   = targetStart + outLen/2;
	const UTF8*  sourceEnd = NULL;

	if ((NULL == source) || (NULL == targetStart)){
		printf("ERR, Utf8_To_Utf16: source=%p, targetStart=%p\n", source, targetStart);
		return conversionFailed;
	}
	sourceEnd   = strlen((const char*)sourceStart) + sourceStart;

	while (*source){
		UTF32 ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd){
			printf("ERR, Utf8_To_Utf16----sourceExhausted: source=%p, extraBytesToRead=%d, sourceEnd=%p\n", source, extraBytesToRead, sourceEnd);
			result = sourceExhausted;
			break;
		}
		/* Do this check whether lenient or strict */
		if (! isLegalUTF8(source, extraBytesToRead+1)){
			printf("ERR, Utf8_To_Utf16----isLegalUTF8 return FALSE: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
			result = sourceIllegal;
			break;
		}
		/*
		 * The cases all fall through. See "Note A" below.
		 */
		switch (extraBytesToRead) {
			case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
			case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
			case 3: ch += *source++; ch <<= 6;
			case 2: ch += *source++; ch <<= 6;
			case 1: ch += *source++; ch <<= 6;
			case 0: ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (target >= targetEnd) {
			source -= (extraBytesToRead+1); /* Back up source pointer! */
			printf("ERR, Utf8_To_Utf16----target >= targetEnd: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
			result = targetExhausted;
			break;
		}
		if (ch <= UNI_MAX_BMP){
			/* Target is a character <= 0xFFFF */
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END){
				if (flags == strictConversion){
					source -= (extraBytesToRead+1); /* return to the illegal value itself */
					printf("ERR, Utf8_To_Utf16----ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
					result = sourceIllegal;
					break;
				} else {
					*target++ = UNI_REPLACEMENT_CHAR;
				}
			} else{
				*target++ = (UTF16)ch; /* normal case */
			}
		}else if (ch > UNI_MAX_UTF16){
			if (flags == strictConversion) {
				result = sourceIllegal;
				source -= (extraBytesToRead+1); /* return to the start */
				printf("ERR, Utf8_To_Utf16----ch > UNI_MAX_UTF16: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
				break; /* Bail out; shouldn't continue */
			} else {
				*target++ = UNI_REPLACEMENT_CHAR;
			}
		} else {
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			if (target + 1 >= targetEnd) {
				source -= (extraBytesToRead+1); /* Back up source pointer! */
				printf("ERR, Utf8_To_Utf16----target + 1 >= targetEnd: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
				result = targetExhausted; break;
			}
			ch -= halfBase;
			*target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
			*target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
		}
	}
	return result;
}

int Utf16_To_Utf8 (const UTF16* sourceStart, UTF8* targetStart, size_t outLen ,  ConversionFlags flags)
{
	int result = 0;
	const UTF16* source = sourceStart;
	UTF8* target        = targetStart;
	UTF8* targetEnd     = targetStart + outLen;

	if ((NULL == source) || (NULL == targetStart)){
		printf("ERR, Utf16_To_Utf8: source=%p, targetStart=%p\n", source, targetStart);
		return conversionFailed;
	}

	while ( *source ) {
		UTF32 ch;
		unsigned short bytesToWrite = 0;
		const UTF32 byteMask = 0xBF;
		const UTF32 byteMark = 0x80;
		const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
		ch = *source++;
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
			/* If the 16 bits following the high surrogate are in the source buffer... */
			if ( *source ){
				UTF32 ch2 = *source;
				/* If it's a low surrogate, convert to UTF32. */
				if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
					ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
					++source;
				}else if (flags == strictConversion) { /* it's an unpaired high surrogate */
					--source; /* return to the illegal value itself */
					result = sourceIllegal;
					break;
				}
			} else { /* We don't have the 16 bits following the high surrogate. */
				--source; /* return to the high surrogate */
				result = sourceExhausted;
				break;
			}
		} else if (flags == strictConversion) {
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END){
				--source; /* return to the illegal value itself */
				result = sourceIllegal;
				break;
			}
		}
		/* Figure out how many bytes the result will require */
		if(ch < (UTF32)0x80){
			bytesToWrite = 1;
		} else if (ch < (UTF32)0x800) {
			bytesToWrite = 2;
		} else if (ch < (UTF32)0x10000) {
			bytesToWrite = 3;
		} else if (ch < (UTF32)0x110000){
			bytesToWrite = 4;
		} else {
			bytesToWrite = 3;
			ch = UNI_REPLACEMENT_CHAR;
		}

		target += bytesToWrite;
		if (target > targetEnd) {
			source = oldSource; /* Back up source pointer! */
			target -= bytesToWrite; result = targetExhausted; break;
		}
		switch (bytesToWrite) { /* note: everything falls through. */
			case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
			case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
			case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
			case 1: *--target = (UTF8)(ch | firstByteMark[bytesToWrite]);
		}
		target += bytesToWrite;
	}
	return result;
}


//读取身份证信息命令交互
//cmd:命令码  in_data:输入数据   in_len:输入数据的长度     out_data:输出数据   out_len:输出数据的长度
int idcard_comm_exchange(int fd,uint8_t *cmd, uint8_t *in_data, int in_len, uint8_t *out_data, int out_len)
{
	uint8_t buf[MAXSIZE] = {0}, *ptr = NULL;
	int len = -1;
	int ret;

	if (cmd == NULL || out_data == NULL || out_len <=0 || out_len <= 0)
		return IDCARD_DATA_NULL;

	/*1.主机通过串口向模块发送命令*/
	ptr = buf;//指向缓冲区
	len = FRAME_OF_CMD_LEN + in_len + FRAME_OF_LRC_LEN;//长度 = 命令码长度（2 命令+参数） + n字节输入数据(0)  + 异或和校验长度  
	memcpy(ptr, FRAME_OF_HEAD, FRAME_OF_HEAD_LEN);//得到帧头ptr = "AA AA AA 96 69"
	ptr += FRAME_OF_HEAD_LEN;  //移到帧头之后
	*ptr++ = (len >> 8) & 0xFF;//得到一字节数据(长度高位)指针移动一个字节长度
	*ptr++ = len & 0xFF;       //得到一字节数据(长度地位)指针移动一个字节长度
	memcpy(ptr, cmd, FRAME_OF_CMD_LEN);//得到命令码  该命令码为获取身份证信息  明文数据
	ptr += FRAME_OF_CMD_LEN;           //指针移动2个字节

	//处理数据字节
	if (in_data != NULL  && in_len > 0) {
		memcpy(ptr, in_data, in_len);
		ptr += in_len;
	}

	//处理异或和校验 得到校验和
	*ptr++ = calc_lrc(&buf[FRAME_OF_HEAD_LEN], FRAME_OF_LENS + len - FRAME_OF_LRC_LEN);

	//打印数据
	for (int i = 0; i < 10; i++)
		printf("buf[%d] = 0x%02X\n",i,buf[i]);

	//sleep(10);
	//写入串口 发送给模块
	ret = uart_send(fd,buf, 10); 
	Delay_ms(8000);

	if (ret < 0) {
		return IDCARD_DATA_ERR_WRITE;
	}

	printf("receive start\n");	
	//清串口输入缓存
	tcflush(fd,TCIFLUSH);
	/*模块发回应答帧(程序需要模拟) 主机通过串口读取并进行解析*/
	uint8_t match[FRAME_OF_HEAD_LEN];//帧头
	memcpy(match, FRAME_OF_HEAD, FRAME_OF_HEAD_LEN);//拷贝帧头到match
	int match_index = 0;

	//循环等待模块响应返回应答帧帧头 并进行比较
	printf("please input resp code:\n");
	memset(buf,0,sizeof(buf));
	sleep(10);
	Delay_ms(5000);
	while (1) {
		printf("call %s line = %d\n",__FUNCTION__,__LINE__);
		ret = uart_read(fd, buf, 1, 3000);
		if (ret < 0)  {
			return IDCARD_DATA_ERR_READ;
			break;
		}
		sleep(5);
		//检查帧头
		if (match[match_index] == buf[0]) 
		{
			match_index++;
			if (match_index >= FRAME_OF_HEAD_LEN) 
				break;
		} 
		else  
		{
			match_index = 0;
		}
	}

	//读取应答帧的长度  2个字节  buf[0]  buf[1]
	ret = uart_read(fd, buf, FRAME_OF_LENS , 500);
	printf("ret = %d\n",ret);
	if (ret < 0) 
	{
		return IDCARD_DATA_ERR_READ;
	}

	len = (buf[0] << 8) + buf[1];
	printf("len = %d \n",len);
	if (len > sizeof(buf) - FRAME_OF_LENS - 1)
		return IDCARD_DATA_ERR_READ;

	if (len < FRAME_OF_RESP_LEN + FRAME_OF_LRC_LEN)
		return IDCARD_DATA_ERR_READ;

	//读取应答码 + n字节数据 + 异或和校验的所有数据
	ret = uart_read(fd,&buf[2], len , 4000);
	if (ret <0) {
		return IDCARD_DATA_ERR_READ;
	}

	for(int i=0;i<len + 2;i++)
		printf("read resp buf[%d]= 0x%02X  ",i,buf[i]);
	printf("\n");

	//计算异或和校验
	if (calc_lrc(buf, len + FRAME_OF_LENS) != 0) {
		return IDCARD_DATA_ERR_LRC;
	}

	printf("返回返回\n");
	sleep(1);
	//应答码处理
	if (!memcmp(&buf[2], CMD_IDC_RESP_EXEC_OK, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_READ_OK;//身份证信息读取成功
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_DETECT_OK, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ACTIVE_OK;//身份证信息激活成功
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_LRC, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_LRC;//接收上位机数据的校验和错
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_LEN, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_LEN;//接收上位机数据的长度错
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_CMD, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_CMD;//接收上位机命令错
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_OTHERS, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_OTHER;//其它不可识别的错误
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_READ, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_READ;//读取身份证信息失败
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_DETECT, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_ACTIVE; //激活身份证失败
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_SELECT, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_SELECT; //选择身份证失败
	}

	printf("ret resp = %d \n",ret);
	if (ret < 0)
		return ret;

	//数据处理
	out_len = len - FRAME_OF_RESP_LEN - FRAME_OF_LRC_LEN;//数据的长度
	if (out_data != NULL && out_len > 0) {
		memcpy(out_data, &buf[FRAME_OF_LENS + FRAME_OF_RESP_LEN], out_len);//将解析完成的数据内存拷贝到out_data中
	}

	printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
	return ret;
}


//激活身份证
int idcard_active(int fd)
{
	uint8_t resp[32];
	int ret = idcard_comm_exchange(fd,(uint8_t*)CMD_IDC_ACTIVEED, NULL, 0, resp,  sizeof(resp));
	if(ret < 0)
	{
		printf("身份证激活失败\n");
		ret = IDCARD_ACTIVE_FAILED;
	}
	else if(ret == IDCARD_DATA_ACTIVE_OK)
	{
		printf("身份证激活成功");
		ret = IDCARD_ACTIVE_OK;
	}

	return ret;
}

//选择身份证
int idcard_select(int fd)
{
	uint8_t resp[32];
	int ret = idcard_comm_exchange(fd,(uint8_t*)CMD_IDC_SELECT, NULL, 0, resp,  sizeof(resp));
	if(ret < 0)
	{
		printf("身份证选择失败\n");
		ret = IDCARD_SELECT_FAILED;
	}
	else if(ret == IDCARD_DATA_READ_OK)
	{
		printf("身份证选择成功");
		ret = IDCARD_SELECT_OK;
	}

	return ret;
}


//读取身份证信息
int idcard_read(int fd,uint8_t *info, int info_len, uint8_t *bitmap, int bitmap_len)
{
	uint8_t resp[4096];
	int ret;

	if (info == NULL || info_len <=0 || bitmap == NULL || bitmap_len <= 0)
		return IDCARD_DATA_NULL;

	ret = idcard_comm_exchange(fd,(uint8_t *)CMD_IDC_READ_PLAIN, NULL, 0, resp, sizeof(resp));//通过身份证读取命令  模块返回响应的数据  拷贝到resp变量

	if (ret < 0)
	{
		return IDCARD_READ_FAILED;
	}
	else if(ret == IDCARD_DATA_READ_OK)
	{
		ret = IDCARD_READ_OK;
		printf("身份证信息读取成功\n");
	}

	info_len = (resp[0] << 8) + resp[1];//数据文字信息长度
	bitmap_len = (resp[2] << 8) + resp[3];//数据图片信息长度
	printf("文字信息长度:%d  图片信息长度:%d\n",info_len,bitmap_len);

	if (info_len + bitmap_len < MINSIZE) {
		return IDCARD_DATA_ERR_LEN;
	}

	if (info_len + bitmap_len > MAXSIZE) {
		return IDCARD_DATA_ERR_LEN;
	}

	printf("拷贝拷贝\n");
	memcpy(info, &resp[4], info_len);//将文字信息拷贝到info中
	memcpy(bitmap, &resp[4 + info_len], bitmap_len);//将图片信息拷贝到bitmap中

	return ret;
}

int idcard_protcol_local(int fd,uint8_t *info, int info_len, uint8_t *bitmap, int bitmap_len)  //通过本地协议解析
{
	int ret;
	//1.激活身份证
	ret = idcard_active(fd);
	if(ret < 0)
		return IDCARD_DATA_ERR_ACTIVE;
	//2.选择身份证
	ret = idcard_select(fd);
	if(ret < 0)
		return IDCARD_DATA_ERR_SELECT;
	//3.读取身份证信息
	ret = idcard_read(fd,info, sizeof(info), bitmap, sizeof(bitmap));
	printf("%s ret = %d\n",__FUNCTION__,ret);
	if(ret < 0)
		return IDCARD_DATA_ERR_READ;

	ret = IDCARD_DATA_READ_OK;

	return ret;
}

int getInfoLenth(char* info)
{
	int i=0;
	while(i<35)//身份证最大字段长度为70
	{
		if((info[2*i]==0x20) && (info[2*i+1]==0x00))
		{
			return i*3;
		}
		i++;
	}
	return 0;
}

//得到姓名
void getName(St_IDCardData info,char* Name)
{
	int lenth = getInfoLenth((char*)info.name);
	Utf16_To_Utf8 ((const UTF16*)info.name, (UTF8*)Name,lenth , strictConversion);
}

//得到性别
void getGender(St_IDCardData info,char* Gender)
{
	char GenderCode[1];
	memset(GenderCode,0x0,1);
	Utf16_To_Utf8 ((const UTF16*)info.gender, (UTF8*)GenderCode,1 , strictConversion);
	int genderCode = atoi(GenderCode);
	if(genderCode==1)
	{
		memcpy(Gender,"男",strlen("男"));
	}
	else
	{
		memcpy(Gender,"女",strlen("男"));
	}
}

//得到民族
void getNational(St_IDCardData info,char*National)
{
	char NationalCode[2] = {0};
	Utf16_To_Utf8 ((const UTF16*)info.national, (UTF8*)NationalCode,2 , strictConversion);
	int nationalCode = atoi(NationalCode);
	memcpy(National,nations[nationalCode],strlen(nations[nationalCode]));
}

//得到出生日期
void getBirthday(St_IDCardData info,char*Birthday)
{
	Utf16_To_Utf8 ((const UTF16*)info.birthday, (UTF8*)Birthday,8, strictConversion);
}

//地址
void getAddress(St_IDCardData info,char*Address)
{
	int lenth = getInfoLenth((char*)info.address);
	Utf16_To_Utf8 ((const UTF16*)info.address,(UTF8*)Address,lenth , strictConversion);
}


//得到身份证号码
void getIDCardNumber(St_IDCardData info,char*IDCardNumber)
{
	Utf16_To_Utf8 ((const UTF16*)info.idnumber, (UTF8*)IDCardNumber,18 , strictConversion);
}

//签发机关
void getMaker(St_IDCardData info,char*Maker)
{
	int lenth = getInfoLenth((char*)info.maker);
	Utf16_To_Utf8 ((const UTF16*)info.maker, (UTF8*)Maker,lenth , strictConversion);
}

//有效起始日期
void getStartDate(St_IDCardData info,char*StartDate)
{
	Utf16_To_Utf8 ((const UTF16*)info.start_date, (UTF8*)StartDate,8 , strictConversion);
}

//有效截止日期
void getEndDate(St_IDCardData info,char*EndDate)
{
	Utf16_To_Utf8 ((const UTF16*)info.end_date, (UTF8*)EndDate,8 , strictConversion);
}

//身份证解析函数
void idcardinfo_anylys(int fd,uint8_t *info, uint8_t *bitmap)
{
	printf("进行解析\n");

	//1.构造身份证文字信息结构体
	St_IDCardData idcard_info_Hex;
	memcpy(&idcard_info_Hex,info,sizeof(St_IDCardData));
	memset(&idcard_info,0,sizeof(St_IDCardData));

	//2.调用编码转换函数解析UFT16编码数据 并显示
	getName(idcard_info_Hex,(char*)idcard_info.name);
	printf("idcard_info.name = %s\n",idcard_info.name);

	getGender(idcard_info_Hex,(char*)idcard_info.gender);
	printf("idcard_info.gender = %s\n",idcard_info.gender);

	getNational(idcard_info_Hex,(char*)idcard_info.national);
	printf("idcard_info.national = %s\n",idcard_info.national);

	getBirthday(idcard_info_Hex,(char*)idcard_info.birthday);
	printf("idcard_info.birthday = %s\n",idcard_info.birthday);

	getAddress(idcard_info_Hex,(char*)idcard_info.address);
	printf("idcard_info.address = %s\n",idcard_info.address);

	getIDCardNumber(idcard_info_Hex,(char*)idcard_info.idnumber);
	printf("idcard_info.idnumber = %s\n",idcard_info.idnumber);

	getMaker(idcard_info_Hex,(char*)idcard_info.maker);
	printf("idcard_info.maker = %s\n",idcard_info.maker);

	getStartDate(idcard_info_Hex,(char*)idcard_info.start_date);
	printf("idcard_info.start_date = %s\n",idcard_info.start_date);

	getEndDate(idcard_info_Hex,(char*)idcard_info.end_date);
	printf("idcard_info.end_date = %s\n",idcard_info.end_date);

	//3.将二进制图片信息转为图片(.wlt--->.jpg / .bmp)  解码算法
	/*	int result = saveWlt2BmpUseFork(id_bitmap, "/userdata/photo.bmp");//解码照片
		if (result == 1) {
		printf("saveWlt2BmpUseFork success\r\n");
		} else {
		printf("saveWlt2BmpUseFork(id_bitmap) = %d\r\n", result);
		}
		*/
}

//动态库主接口函数
int idcardregnton(uint8_t** ptr_idcard_info,uint8_t** ptr_idcard_bitmap,St_IDCardData** ptr_st_idcard_info)
{

	int   ret;
	int   card_type = NODETECT;
	
	//初始化串口
	int fd = uart_init();
	if(fd <0)
		printf("初始化串口失败\n");
	else
		printf("初始化串口成功\n");
	
	sleep(10);
	
	//循环处理模块发送的命令
	while (1) 
	{
		printf("start loop\n");
		//1.检查是否有卡靠近模块(射频设备) 如果有卡片,则解析模块向串口输出的数据,返回卡片类型信息
		card_type = card_detect(fd,cardread_status);

		if (card_type == NODETECT) //未检测到卡片 继续下一次循环检测
		{
			cardread_status = CARDREAD_STATUS_NO;
			printf("NO DETECT CARD\n");
			sleep(10);
			continue;
		}
		else
		{
			printf("HAVING DETECTED CARD\n");	
			tcflush(fd,TCIFLUSH);//检测到卡片 清空串口输入
			cardread_status = CARDREAD_STATUS_ING;//读卡中
			sleep(10);
			//检查卡片是否快速移开
			bool is_moved = is_cardMoved(fd);
			if(is_moved == true)
			{
				printf("卡片被移除了\n");
				cardread_status = CARDREAD_STATUS_MOVED;
			}
			else
			{
				printf("卡片未被移除\n");
			}

			/*
			//创建卡片移开线程  实时监控卡片是否移除
			ret  = pthread_create(&cardmoved_pid, NULL, pthread_cardmoved,(void *)&fd);
			if(ret <0)
			{
			perror("pthread create");
			continue;
			}
			pthread_detach(pthread_self());//分离状态  线程结束后自动释放线程资源
			*/
			if(cardread_status == CARDREAD_STATUS_ING)
			{
				//读取卡片信息
				ret = cardinfo_read(fd,cardread_status,card_type);//处理卡片信息
				printf("读卡 ret = %d\n",ret);
				if(ret < 0)
				{
					printf("读卡失败");
					cardread_status = CARDREAD_STATUS_FAILED;
					continue;
				}

				cardread_status = CARDREAD_STATUS_OK;//读卡成功
				printf("读卡成功\n");	
			}
			else if(cardread_status == CARDREAD_STATUS_MOVED)
			{
				printf("请重新放入身份证\n");
				cardread_status = CARDREAD_STATUS_NO;
				tcflush(fd,TCIOFLUSH);
				continue;
			}

			//3.卡片信息读取成功  解析卡片信息
			cardinfo_anylys(fd,cardread_status,card_type);
			if(cardread_status == CARDREAD_STATUS_OK)
				break;
		}
	}//end while

	
	//拷贝全局变量的数值给相应的指针
	*ptr_idcard_info = info;
	*ptr_idcard_bitmap = bitmap;
	*ptr_st_idcard_info = &idcard_info;



	//关闭串口
	uart_close(fd);
	return 0;
}
