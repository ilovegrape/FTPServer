all:ftpserver

CC=gcc
CFLAGS=-D DEBUG
LDFLAGS= -lpthread
OBJS=main.o

ftpserver:$(OBJS)
	$(CC) $< -o $@ $(LDFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@



clean:
	@rm -f *.o
	@rm ftpserver


