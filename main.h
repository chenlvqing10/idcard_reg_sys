#ifndef __MAIN__
#define __MAIN__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "type.h"

/*描述卡片状态*/
enum CARDREAD_STATUS {
    CARDREAD_STATUS_NO  = 0,//未读卡
    CARDREAD_STATUS_FAILED,//读卡失败
	CARDREAD_STATUS_ING,//读卡中
	CARDREAD_STATUS_MOVED,//卡片移开
    CARDREAD_STATUS_OK//读卡成功
};


#endif
