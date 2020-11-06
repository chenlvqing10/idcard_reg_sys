
OBJS=main.c  uart.c card.c  idcard.c  utf.c server_socket.c
CC=aarch64-linux-gnu-gcc
#CC=arm-linux-gnueabihf-gcc
ifeq ($(CC),aarch64-linux-gnu-gcc)
TARGET=id_card_arm64
else
TARGET=id_card_arm32
endif
CFLAGS+=-c -Wall -g

ifeq ($(CC),aarch64-linux-gnu-gcc)
 $(TARGET):$(OBJS)
	$(CC) $^ -o  $@ -lpthread
else
 $(TARGET):$(OBJS)
	$(CC) $^ -o  $@ -lpthread -lSynReaderArm -lwlt -lusb-1.0
endif
%.o:%.c
	$(CC) $^ $(CFLAGS) -o $@
clean:
	$(RM) *.o $(TARGET) -r
copy:
	cp $(TARGET)  /mnt/hgfs/Share/

