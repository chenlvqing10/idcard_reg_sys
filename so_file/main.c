#include "idcardregnton.h"


int main()
{
	uint8_t *id_info = (uint8_t *)malloc(256);
	uint8_t *id_bitmap = (uint8_t *)malloc(1024);
	St_IDCardData *st_id_info = (St_IDCardData *) malloc(sizeof(St_IDCardData));

	idcardregnton(&id_info,&id_bitmap,&st_id_info);
	sleep(10);
	printf("main main mian\n");	

	for(int i=0;i<256;i++)
		printf("0x%02X ",id_info[i]);

	return 0;
}
