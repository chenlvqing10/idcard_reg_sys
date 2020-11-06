#include "idcard.h"
#include "card.h"
#include "uart.h"
#include "utf.h"

/* 读取身份证信息命令交互
 *cmd:命令码  in_data:输入数据   in_len:输入数据的长度     out_data:输出数据   out_len:输出数据的长度
 *返回:应答状态
*/
int32_t idcard_comm_exchange(int32_t fd,uint8_t *cmd, uint8_t *in_data, int32_t in_len, uint8_t *out_data, int32_t out_len)
{
	uint8_t buf[MAXSIZE] = {0}, *ptr = NULL;
	int32_t len = -1;
	int32_t ret;

	if (cmd == NULL || out_data == NULL || out_len <=0 || out_len <= 0)
		return IDCARD_DATA_NULL;

	/* 1.主机通过串口向模块发送命令 */
	ptr = buf;
	len = FRAME_OF_CMD_LEN + in_len + FRAME_OF_LRC_LEN;/* 长度 = 命令码长度（2 命令+参数） + n字节输入数据(0)  + 异或和校验长度 */
	memcpy(ptr, FRAME_OF_HEAD, FRAME_OF_HEAD_LEN);/* 得到帧头ptr = "AA AA AA 96 69" */
	ptr += FRAME_OF_HEAD_LEN;  /* 移到帧头之后 */
	*ptr++ = (len >> 8) & 0xFF;/* 得到一字节数据(长度高位)指针移动一个字节长度 */
	*ptr++ = len & 0xFF;       /* 得到一字节数据(长度地位)指针移动一个字节长度 */
	memcpy(ptr, cmd, FRAME_OF_CMD_LEN); /* 得到命令码  该命令码为获取身份证信息  明文数据 */
	ptr += FRAME_OF_CMD_LEN;            /*指针移动2个字节 */

	/* 处理数据字节 */
	if (in_data != NULL  && in_len > 0) {
		memcpy(ptr, in_data, in_len);
		ptr += in_len;
	}

	/* 处理异或和校验 得到校验和 */
	*ptr++ = calc_lrc(&buf[FRAME_OF_HEAD_LEN], FRAME_OF_LENS + len - FRAME_OF_LRC_LEN);

	/* 打印数据 debug print*/
	for (int32_t i = 0; i < 10; i++)
		printf("buf[%d] = 0x%02X\n",i,buf[i]);

	/* 写入串口 发送给模块 write to serial*/
	ret = uart_send(fd,buf, 10); 

	if (ret < 0) {
		return IDCARD_DATA_ERR_WRITE;
	}

	printf("receive start\n");	
	
	/* 清串口输入缓存 clear serial input buffer to get true resp code*/
	tcflush(fd,TCIFLUSH);

	/* 模块发回应答帧(程序需要模拟) 主机通过串口读取并进行解析 */
	uint8_t match[FRAME_OF_HEAD_LEN];
	memcpy(match, FRAME_OF_HEAD, FRAME_OF_HEAD_LEN);/* 拷贝帧头到match check the resp code frame head */
	int32_t match_index = 0;
	/* 循环等待模块响应返回应答帧帧头 并进行比较 */
	printf("please input resp code:\n");
	memset(buf,0,sizeof(buf));
	sleep(10);
	Delay_ms(5000);
	while (1) {
		//printf("call %s line = %d\n",__FUNCTION__,__LINE__);
		ret = uart_read(fd, buf, 1, 3000);
		if (ret < 0) {
			return IDCARD_DATA_ERR_READ;
			break;
		}
		//sleep(5);
		/* 检查帧头 by one byte once*/
		if (match[match_index] == buf[0]) {
			match_index++;
			if (match_index >= FRAME_OF_HEAD_LEN) 
				break;
		} 
		else {
			match_index = 0;
		}
	}

	/* 读取应答帧的长度  2个字节  buf[0]  buf[1] */
	ret = uart_read(fd, buf, FRAME_OF_LENS , 500);
	//printf("ret = %d\n",ret);
	if (ret < 0) {
		return IDCARD_DATA_ERR_READ;
	}

	len = (buf[0] << 8) + buf[1];
	printf("len = %d \n",len);
	if (len > sizeof(buf) - FRAME_OF_LENS - 1)
		return IDCARD_DATA_ERR_READ;

	if (len < FRAME_OF_RESP_LEN + FRAME_OF_LRC_LEN)
		return IDCARD_DATA_ERR_READ;

	/* 读取应答码 + n字节数据 + 异或和校验的所有数据 */
	ret = uart_read(fd,&buf[2], len , 4000);
	if (ret <0) {
		return IDCARD_DATA_ERR_READ;
	}

	/*
	for(int i=0;i<len + 2;i++)
		printf("read resp buf[%d]= 0x%02X  ",i,buf[i]);
	printf("\n");
	*/

	//计算异或和校验
	if (calc_lrc(buf, len + FRAME_OF_LENS) != 0) {
		return IDCARD_DATA_ERR_LRC;
	}

	//printf("返回返回\n");
	//sleep(1);
	/* 应答码处理 check the resp code*/
	if (!memcmp(&buf[2], CMD_IDC_RESP_EXEC_OK, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_READ_OK; /* 身份证信息读取成功 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_DETECT_OK, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ACTIVE_OK;/* 身份证信息激活成功 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_LRC, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_LRC;/* 接收上位机数据的校验和错 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_LEN, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_LEN;/* 接收上位机数据的长度错 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_CMD, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_CMD;/* 接收上位机命令错 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_OTHERS, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_OTHER;/* 其它不可识别的错误 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_READ, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_READ;/* 读取身份证信息失败 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_DETECT, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_ACTIVE; /* 激活身份证失败 */
	}
	else if (!memcmp(&buf[2], CMD_IDC_RESP_ERR_SELECT, FRAME_OF_RESP_LEN)) 
	{
		ret = IDCARD_DATA_ERR_SELECT; /* 选择身份证失败 */
	}

	printf("ret resp = %d \n",ret);
	if (ret < 0)
		return ret;

	/* 数据处理 将解析完成的数据内存拷贝到out_data */
	out_len = len - FRAME_OF_RESP_LEN - FRAME_OF_LRC_LEN;
	if (out_data != NULL && out_len > 0) {
		memcpy(out_data, &buf[FRAME_OF_LENS + FRAME_OF_RESP_LEN], out_len);
	}

	return ret;
}


/* 激活身份证 */
int32_t idcard_active(int32_t fd)
{
	uint8_t resp[32];
	int32_t ret = idcard_comm_exchange(fd,(uint8_t*)CMD_IDC_ACTIVEED, NULL, 0, resp,  sizeof(resp));
	
	if(ret == IDCARD_DATA_ACTIVE_OK) {
		printf("身份证激活成功");
		ret = IDCARD_ACTIVE_OK;
	}
	else {
		printf("身份证激活失败\n");
		ret = IDCARD_ACTIVE_FAILED;
	}

	return ret;
}

/* 选择身份证 */
int32_t idcard_select(int32_t fd)
{
	uint8_t resp[32];
	int32_t ret = idcard_comm_exchange(fd,(uint8_t*)CMD_IDC_SELECT, NULL, 0, resp,  sizeof(resp));
	
	if(ret == IDCARD_DATA_READ_OK) {
		printf("身份证选择成功");
		ret = IDCARD_SELECT_OK;
	}
	else {
		printf("身份证选择失败\n");
		ret = IDCARD_SELECT_FAILED;
	}

	return ret;
}


/* 读取身份证信息 */
int32_t idcard_read(int32_t fd,uint8_t *info, int32_t info_len, uint8_t *bitmap, int32_t bitmap_len)
{
	uint8_t resp[4096];
	int32_t ret;

	if (info == NULL || info_len <=0 || bitmap == NULL || bitmap_len <= 0)
		return IDCARD_DATA_NULL;

	ret = idcard_comm_exchange(fd,(uint8_t *)CMD_IDC_READ_PLAIN, NULL, 0, resp, sizeof(resp)); /* 通过身份证读取命令  模块返回响应的数据  拷贝到resp变量 */

	if (ret == IDCARD_DATA_READ_OK) {
		ret = IDCARD_READ_OK;
		printf("身份证信息读取成功\n");
	}
	else {
		return IDCARD_READ_FAILED;
	}

	info_len = (resp[0] << 8) + resp[1];   /* 数据文字信息长度 */
	bitmap_len = (resp[2] << 8) + resp[3]; /* 数据图片信息长度 */
	printf("文字信息长度:%d  图片信息长度:%d\n",info_len,bitmap_len);

	if (info_len + bitmap_len < MINSIZE) {
		return IDCARD_DATA_ERR_LEN;
	}

	if (info_len + bitmap_len > MAXSIZE) {
		return IDCARD_DATA_ERR_LEN;
	}

	//printf("拷贝拷贝\n");
	memcpy(info, &resp[4], info_len);  /* 将文字信息拷贝到info中 */
	memcpy(bitmap, &resp[4 + info_len], bitmap_len); /* 将图片信息拷贝到bitmap中 */

	return ret;
}

/* 通过本地协议解析 */
int32_t idcard_protcol_local(int32_t fd,uint8_t *info, int32_t info_len, uint8_t *bitmap, int32_t bitmap_len)
{
	int32_t ret;
	/* 1.激活身份证 */
	ret = idcard_active(fd);
	if(ret < 0)
		return IDCARD_DATA_ERR_ACTIVE;

	/*2.选择身份证 */
	ret = idcard_select(fd);
	if(ret < 0)
		return IDCARD_DATA_ERR_SELECT;

	/* 3.读取身份证信息 */
	ret = idcard_read(fd,info, sizeof(info), bitmap, sizeof(bitmap));
	//printf("%s ret = %d\n",__FUNCTION__,ret);
	if(ret < 0)
		return IDCARD_DATA_ERR_READ;

	ret = IDCARD_DATA_READ_OK;

	return ret;
}

/* 得到实际的字节长度 */
int32_t getInfoLenth(char* info)
{
	int32_t i=0;
	while(i<35) { /* 身份证最大字段长度为70字节 70/2 = 35 */
		if((info[2*i]==0x20) && (info[2*i+1]==0x00)) {
			return i*3;
		}
		i++;
	}
	return 0;
}

/* 得到姓名 */
void getName(St_IDCardData info,char* Name)
{
	int32_t lenth = getInfoLenth((char*)info.name);
	Utf16_To_Utf8 ((const UTF16*)info.name, (UTF8*)Name,lenth , strictConversion);
}

/* 得到性别 */
void getGender(St_IDCardData info,char* Gender)
{
	char GenderCode[1];
	memset(GenderCode,0x0,1);
	Utf16_To_Utf8 ((const UTF16*)info.gender, (UTF8*)GenderCode,1 , strictConversion);
	int32_t genderCode = atoi(GenderCode);
	if(genderCode==1) {
		memcpy(Gender,"男",strlen("男"));
	}
	else {
		memcpy(Gender,"女",strlen("男"));
	}
}

/* 得到民族 */
void getNational(St_IDCardData info,char*National)
{
	char NationalCode[2] = {0};
	Utf16_To_Utf8 ((const UTF16*)info.national, (UTF8*)NationalCode,2 , strictConversion);
	int32_t nationalCode = atoi(NationalCode);
	memcpy(National,nations[nationalCode],strlen(nations[nationalCode]));
}

/* 得到出生日期 */
void getBirthday(St_IDCardData info,char*Birthday)
{
	Utf16_To_Utf8 ((const UTF16*)info.birthday, (UTF8*)Birthday,8, strictConversion);
}

/* 得到住址 */
void getAddress(St_IDCardData info,char*Address)
{
	int32_t lenth = getInfoLenth((char*)info.address);
	Utf16_To_Utf8 ((const UTF16*)info.address,(UTF8*)Address,lenth , strictConversion);
}

/* 得到身份证号码 */
void getIDCardNumber(St_IDCardData info,char*IDCardNumber)
{
	Utf16_To_Utf8 ((const UTF16*)info.idnumber, (UTF8*)IDCardNumber,18 , strictConversion);
}

/* 得到签发机关 */
void getMaker(St_IDCardData info,char*Maker)
{
	int32_t lenth = getInfoLenth((char*)info.maker);
	Utf16_To_Utf8 ((const UTF16*)info.maker, (UTF8*)Maker,lenth , strictConversion);
}

/* 得到有效起始日期 */
void getStartDate(St_IDCardData info,char*StartDate)
{
	Utf16_To_Utf8 ((const UTF16*)info.start_date, (UTF8*)StartDate,8 , strictConversion);
}

/* 得到有效截止日期 */
void getEndDate(St_IDCardData info,char*EndDate)
{
	Utf16_To_Utf8 ((const UTF16*)info.end_date, (UTF8*)EndDate,8 , strictConversion);
}

/* 身份证解析函数 */
void idcardinfo_anylys(int fd,uint8_t *info, uint8_t *bitmap)
{
	//printf("进行解析\n");

	/* 1.构造身份证文字信息结构体 */
	St_IDCardData idcard_info_Hex;
	memcpy(&idcard_info_Hex,info,sizeof(St_IDCardData));
	memset(&idcard_info,0,sizeof(St_IDCardData));

	/* 2.调用编码转换函数解析UFT16编码数据 并显示 */
	getName(idcard_info_Hex,(char*)idcard_info.name);
	//printf("idcard_info.name = %s\n",idcard_info.name);

	getGender(idcard_info_Hex,(char*)idcard_info.gender);
	//printf("idcard_info.gender = %s\n",idcard_info.gender);

	getNational(idcard_info_Hex,(char*)idcard_info.national);
	//printf("idcard_info.national = %s\n",idcard_info.national);

	getBirthday(idcard_info_Hex,(char*)idcard_info.birthday);
	//printf("idcard_info.birthday = %s\n",idcard_info.birthday);

	getAddress(idcard_info_Hex,(char*)idcard_info.address);
	//printf("idcard_info.address = %s\n",idcard_info.address);

	getIDCardNumber(idcard_info_Hex,(char*)idcard_info.idnumber);
	//printf("idcard_info.idnumber = %s\n",idcard_info.idnumber);

	getMaker(idcard_info_Hex,(char*)idcard_info.maker);
	//printf("idcard_info.maker = %s\n",idcard_info.maker);

	getStartDate(idcard_info_Hex,(char*)idcard_info.start_date);
	//printf("idcard_info.start_date = %s\n",idcard_info.start_date);

	getEndDate(idcard_info_Hex,(char*)idcard_info.end_date);
	//printf("idcard_info.end_date = %s\n",idcard_info.end_date);

#ifdef __FCU2303__ARM64__ /* 如果在ARM64的平台上,需要使用相关的ARM 64位动态库解码  交叉编译 aarch64-linux-gnu-gcc */

#endif

#ifdef  __FCU2303__ARM32__ /* 如果在ARM32的平台上,需要使用相关的ARM 32位动态库解码  arm-linux-gnueabihf-gcc */
	/* 3.将二进制图片信息转为图片(.wlt--->.jpg / .bmp)  解码算法 */
	int result = saveWlt2BmpUseFork((char*)bitmap, "./photo.bmp");//解码照片
	if (result == 1) {
		printf("saveWlt2BmpUseFork success\r\n");
	} 
	else {
		printf("saveWlt2BmpUseFork(id_bitmap) = %d\r\n", result);
	}
#endif

#ifdef   __FCU2303__X64__/* 如果在X64 平台上 需要使用相关的x86 64位动态库解码  gcc */
	/* 3.将二进制图片信息转为图片(.wlt--->.jpg / .bmp)  解码算法 */
	int result = saveWlt2BmpUseFork((char*)bitmap, "./photo.bmp");//解码照片
	if (result == 1) {
		printf("saveWlt2BmpUseFork success\r\n");
	} 
	else {
		printf("saveWlt2BmpUseFork(id_bitmap) = %d\r\n", result);
	}
#endif
}
