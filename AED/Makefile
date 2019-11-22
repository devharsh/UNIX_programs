TARGET=aed
CC=cc
CFLAGS=-ansi -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow -lcrypto
RM=rm -f

default: $(TARGET) blowfish
all: default 

$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o

$(TARGET).o: $(TARGET).c $(TARGET).h
	$(CC) $(CFLAGS) -c $(TARGET).c

blowfish:
	$(CC) $(CFLAGS) -o blowfish sym_funcs.c 

clean:
	$(RM) $(TARGET) blowfish *.o
