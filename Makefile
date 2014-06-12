all:ftpserver

CC=gcc
CFLAGS=-D DEBUG

OBJS=main.o

ftpserver:$(OBJS)
	$(CC) $< -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@



clean:
	@rm -f *.o
	@rm ftpserver


