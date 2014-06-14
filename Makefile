all:ftpserver

CC=gcc
CFLAGS=-D DEBUG
LDFLAGS= -lpthread
OBJS=main.o connection.o command.o

ftpserver:$(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@



clean:
	rm -f *.o
	rm -f ftpserver
	rm -f *.txt

