#DEBUG = -g -O0
DEBUG = -O3
CC = gcc
AS = as
AR = ar
SRC_DIR_0 = /home/pi/Desktop/sdcard/src
INCLUDE = -I/usr/local/include /home/pi/Desktop/sdcard/src
CFLAGS = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe
LDFLAGS = -L/usr/local/lib
LIBS = -wiringpi

# Should not alter anything below this line
##############################################################################
OBJS=$(SRC_DIR_0)/main.c \
	$(SRC_DIR_0)/ff.c \
	$(SRC_DIR_0)/mmcbb.c
all: $(OBJS)
	$(AR) $(OBJS)
	rm -rf $(SRC_DIR_0)/*.o
%.o:%.c $(INCLUDE)
	$(CC) -c -o $@
library:
	$(CC) $(OBJS) -o RpiSDCard $(LDFLAGS) $(LIBS)
.PHONY : clean
clean:
	rm -rf $(SRC_DIR_0)/*.o

      # DO NOT DELETE 
