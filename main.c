#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "connection.h"

char 	g_sz_rootdir[256] = "/home/yaoxin";
int		g_n_port = 21;
int		g_n_count = 5;
//int		data_port = 7788;
int 	listenfd;
int		ftpserver_init(int argc, char** argv);
int		ftpserver_start();
void	ftpserver_stop();

void handle_sigstop(int signo)
{
	printf("sigint catched...\n");
	close(listenfd);
	//kill(getpid(), SIGINT);
	exit(0);
}

int main(int argc, char** argv)
{
	signal(SIGINT, handle_sigstop);
	if(ftpserver_init(argc, argv) != 0)
	{
		printf("ftpserver_init() error...\n");
		return -1;
	}

#ifdef DEBUG
	printf("port: %d, rootdir=%s\n", g_n_port, g_sz_rootdir);
#endif

	ftpserver_start();

	ftpserver_stop();
	return 0;
}

int ftpserver_init(int argc, char** argv)
{
	int opt;	
	while((opt = getopt(argc, argv, "p:n:d:")) != -1)
	{
		switch(opt)
		{
			case 'p':
				g_n_port = atoi(optarg);
				break;
			case 'n':
				g_n_count = atoi(optarg);
				break;
			case 'd':
				strcpy(g_sz_rootdir, optarg);
				break;
			default:
				break;
		}
	}
	return 0;
}

int ftpserver_start()
{
	struct sockaddr_in address;
	int ret;
	//socket
	listenfd = socket(PF_INET, SOCK_STREAM, 0);
	if(listenfd == -1)
	{
		return -1;
	}

	//bind sockaddr_in
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
	address.sin_port = htons(g_n_port);

	//bind
	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	if(-1 == ret)
	{
		printf("bind() error:%s...\n", "...");
		return -2;
	}

	//listen
	ret = listen(listenfd, g_n_count);
	if(-1 == ret)
	{
		printf("listen() error...\n");
		return -3;
	}

	//client sockaddr_in
	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);
	int fd;
	char * reply = NULL;
	//init_data_connection(data_port);
	while((fd = accept(listenfd, (struct sockaddr*)&client_addr, &addr_len)))
	{
		if(fd < 0)
		{
			printf("ERROR[%4d]: %s\n", errno, strerror(errno));
			break;
		}
		int pid = fork();
		if(pid > 0)
		{
			//child process
			ret = process_loop(fd);
		}
		else
		{
			//parent process
			close(fd);
		}
	}
	close(listenfd);
	return 0;
}
void ftpserver_stop()
{
}

