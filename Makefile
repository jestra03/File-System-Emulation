CC = gcc
CFLAGS = -Wall -std=c99 -pedantic
MAIN = fs_emulator
OBJS = fs_emulator.o

all : $(MAIN)

$(MAIN) : $(OBJS) fs_emulator.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

fs_emulator.o : fs_emulator.c fs_emulator.h
	$(CC) $(CFLAGS) -c fs_emulator.c

clean :
	rm *.o $(MAIN) core
