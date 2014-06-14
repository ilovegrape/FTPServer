#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "command.h"
#include "connection.h"

char buffer[2048];
char port_open = 0;
int 	listenfd_data;
int 	clientfd_data;
int data_port = 7788;

int send_rsp(int fd, char* str)
{
	send(fd, str, strlen(str), 0);
}
int handle_cmd_user(int fd, char* buffer)
{
	char *str = "332 \r\n";
	send_rsp(fd, str); 	
}

int handle_cmd_pass(int fd, char* buffer)
{
	char *str = "230 \r\n";
	send_rsp(fd, str); 	
}

int handle_cmd_syst(int fd, char* buffer)
{
	char *str = "215 UNIX system type\r\n";
	send_rsp(fd, str); 	
}

int handle_cmd_pasv(int fd, char* buffer)
{
	if(port_open == 0)
	{
		port_open = 1;
		init_data_connection(7788);
	}

	sprintf(buffer, "227 192,168,4,100,%d,%d\r\n", (data_port >> 8)&0XFF, data_port&0xFF);
	send_rsp(fd, buffer);
}

int handle_cmd_retr(int fd, char* buffer)
{

	sprintf(buffer, "125 \r\n");
	send_rsp(fd, buffer);

	sprintf(buffer, "1234567890\r\n");
	send_rsp(clientfd_data, buffer);
	//close(clientfd_data);

	sprintf(buffer, "226 \r\n");
	send_rsp(fd, buffer);
	//close(clientfd_data);

}

int handle_cmd_port(int fd, char* buffer)
{


}

int handle_cmd_cwd(int fd, char* buffer)
{


}

int process_loop(int fd)
{
	int bytes;
	send_rsp(fd, "220 \r\n"); 
	while(1)
	{
		bytes = recv(fd, buffer, 2047, 0);
		buffer[bytes] = '\0';
		printf("RECV[%4d]: %s", bytes, buffer);
		FTP_CMD cmd = command_parser(buffer);
		switch(cmd)
		{
			case CMD_USER:
				handle_cmd_user(fd, buffer + 5);
				break;
			case CMD_PASS:
				handle_cmd_pass(fd, buffer + 5);
				break;
			case CMD_SYST:
				handle_cmd_syst(fd, buffer + 5);
				break;
			case CMD_PASV:
				handle_cmd_pasv(fd, buffer + 5);
				break;
			case CMD_RETR:
				handle_cmd_retr(fd, buffer + 5);
				break;
			case CMD_PORT:
				handle_cmd_port(fd, buffer + 5);
				break;
			case CMD_QUIT:
				port_open = 0;
				close(fd);
				break;
		}
		if(CMD_QUIT == cmd)
		{
			break;
		}
	}
	printf("process_loop() end...\n");
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
		printf("bind() error when set up data connection...\n");
		close(listenfd_data);
		return -2;
	}
	ret = listen(listenfd_data, 5);
	if(ret < 0)
	{
		printf("listen() error when set up data connection...\n");
		close(listenfd_data);
		return -3;
	}
	struct sockaddr_in client_addr;
	int size = sizeof(client_addr);
	bzero(&client_addr, size);
	//while(1)
	{
		clientfd_data = accept(listenfd_data, (struct sockaddr*)&client_addr, &size);
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
