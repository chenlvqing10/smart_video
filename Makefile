OBJS=*.c
CC=gcc
#CC=aarch64-linux-gnu-gcc
#CC=arm-linux-gnueabihf-gcc
ifeq ($(CC),aarch64-linux-gnu-gcc)
TARGET=video_arm64
endif
ifeq ($(CC),arm-linux-gnueabihf-gcc)
TARGET=video_arm32
endif

ifeq ($(CC),gcc)
TARGET=video
endif

CFLAGS+=-c -Wall -g

$(TARGET):$(OBJS)
	$(CC) $^ -o  $@ -lpthread -ljson-c
%.o:%.c
	$(CC) $^ $(CFLAGS) -o $@
clean:
	$(RM) *.o $(TARGET) -r
copy:
	cp $(TARGET)  /mnt/hgfs/Share/

