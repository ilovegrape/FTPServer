#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

char 	g_sz_rootdir[256] = "/home/yaoxin";
int		g_n_port = 21;
int		g_n_count = 5;
int		data_port = 7788;
int 	listenfd;
int 	clientfd;
int		listenfd_data;
int 	clientfd_data;
int		ftpserver_init(int argc, char** argv);
int		ftpserver_start();
void	ftpserver_stop();

void handle_sigstop(int signo)
{
	printf("sigint catched...\n");
	close(listenfd);
	close(listenfd_data);
	close(clientfd);
	close(clientfd_data);
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
		printf("bind() error...\n");
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
	init_data_connection(data_port);
	while((fd = accept(listenfd, (struct sockaddr*)&client_addr, &addr_len)))
	{
		if(fd < 0)
		{
			printf("ERROR[%4d]: %s\n", errno, strerror(errno));
			break;
		}
		clientfd = fd;
		char buffer[2048];
		reply = "220 \r\n",
		send(fd, reply, strlen(reply) , 0);
		size_t count;
		count = recv(fd, buffer, 2048, 0);
		buffer[count] = '\0';
		printf("recv[%d] : %s\n", count, buffer);
		if(strncmp(buffer, "USER", 4) != 0)
		{
			close(fd);
			continue;
		}
		reply = "332 \r\n";
		send(fd, reply, strlen(reply), 0);
		count = recv(fd, buffer, 2048, 0);
		buffer[count] = '\0';
		printf("recv[%d] : %s\n", count, buffer);
		if(strncmp(buffer, "PASS", 4) != 0)
		{
			close(fd);
			continue;
		}
		reply = "230 \r\n";
		send(fd, reply, strlen(reply), 0);
		count = recv(fd, buffer, 2048, 0);
		buffer[count] = '\0';
		printf("recv[%d] : %s\n", count, buffer);
		if(strncmp(buffer, "SYST", 4) != 0)
		{
			close(fd);
			continue;
		}
		reply = "215 UNIX system type \r\n";
		send(fd, reply, strlen(reply), 0);

		count = recv(fd, buffer, 2048, 0);
		buffer[count] = '\0';
		printf("recv[%d] : %s\n", count, buffer);

		if(strncmp(buffer, "PASV", 4) != 0)
		{
			close(fd);
			continue;
		}
		sprintf(buffer, "227 192,168,4,100,%d,%d\r\n", (data_port >> 8)&0XFF, data_port&0xFF);
		reply = buffer;
		send(fd, reply, strlen(reply), 0);


		count = recv(fd, buffer, 2048, 0);
		buffer[count] = '\0';
		printf("recv[%d] : %s\n", count, buffer);

		if(strncmp(buffer, "LIST", 4) == 0)
		{
	
			sprintf(buffer, "125 \r\n");
			send(fd, reply, strlen(reply), 0);


			sprintf(buffer, "dir1\r\ndir2\r\ndir3\r\n");
			send(clientfd_data, reply, strlen(reply), 0);
			//close(clientfd_data);

			sprintf(buffer, "250 \r\n");
			send(fd, reply, strlen(reply), 0);
			close(clientfd_data);
		}
		else if(strncmp(buffer, "RETR", 4) == 0)
		{

			sprintf(buffer, "125 \r\n");
			send(fd, reply, strlen(reply), 0);


			sprintf(buffer, "1234567890\r\n");
			send(clientfd_data, reply, strlen(reply), 0);
			//close(clientfd_data);

			sprintf(buffer, "250 \r\n");
			send(fd, reply, strlen(reply), 0);
			close(clientfd_data);
		}
		else
		{
			close(fd);
			continue;
		}

		count = recv(fd, buffer, 2048, 0);
		buffer[count] = '\0';
		printf("recv[%d] : %s\n", count, buffer);

		close(fd);


	}
	close(listenfd);
	return 0;
}
void ftpserver_stop()
{
}

void* data_trans_thread(void* param);
int init_data_connection(int port)
{
	pthread_t id;
	pthread_create(&id, NULL, data_trans_thread, (void*)port);

}
void* data_trans_thread(void* param)
{
	int ret;
	struct sockaddr_in svr_addr;
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons((int)param);
	inet_pton(AF_INET, "0.0.0.0", &svr_addr.sin_addr);
	listenfd_data = socket(PF_INET, SOCK_STREAM, 0);
	if(listenfd_data < 0)	return -1;
	ret = bind(listenfd_data, (struct sockaddr *)&svr_addr, sizeof(svr_addr));
	if(ret < 0)
	{
		close(listenfd_data);
		return -2;
	}
	ret = listen(listenfd_data, 5);
	if(ret < 0)
	{
		close(listenfd_data);
		return -3;
	}
	struct sockaddr_in client_addr;
	int size = sizeof(client_addr);
	bzero(&client_addr, size);
	while((clientfd_data = accept(listenfd_data, (struct sockaddr*)&client_addr, &size)) != -1)
	{
		if(clientfd_data < 0)
		{	
			printf("accept client connect error....\n");
		}
		printf("data client connected.....\n");
		//send(clientfd_data, "1234567890", 10, 0);
		//close(clientfd_data);
	}
	close(listenfd_data);
}
