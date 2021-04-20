#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "main.h"
#include <pthread.h>
#include "device.h"
#include "socket.h"
#include <signal.h>
#include <unistd.h>

extern int g_socket_fd;
pthread_mutex_t connect_lock;

//pthread function
int connectToServer(int fd)
{
	printf("connect to server g_socket_fd = %d \n",fd);
	int count = 5,ret;
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;   // IPv4
	server_addr.sin_addr.s_addr=inet_addr(SERVER_IP);//服务器地址
	server_addr.sin_port = htons(SERVER_PORT);   // 端口
	
	while(count--)//connect 5 times
	{
		ret = connect(g_socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
		if(ret == -1)
		{
			printf("connect failed\n");
			continue;
		}
		printf("connect success\n");
		break;
	}
	if(count == 0) {
		printf("connect failed\n");
		return -1;
	}
	
	return 0 ;
}

int InitSocket()
{
	//create socket client
	g_socket_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(-1 == g_socket_fd)
	{
		return FAILURE;
	}
	printf("client socket fd create success\n");

	//地址快速重用
	int b_reuse = 1;
	if(setsockopt(g_socket_fd,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int)) <0){
		perror("setsockopt");
		exit(1);
	}
	printf("----设置地址快速重用成功----\n");
    
	int ret = connectToServer(g_socket_fd);
	if(ret < 0) 
		return FAILURE;
	//void *status;
    //pthread_join(tid,&status);
	return SUCCESS;
}
