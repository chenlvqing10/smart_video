#include <stdio.h>                 
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#define SERVERPORT 8001
#define SERVER_IP "192.168.1.136"

int main() 
{
	int ret;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0) {
		perror("create socket");
		return -1;
	}
	printf("socket create success!!!\n");
	

	//设置本地地址结构体
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));			// 清空,保证最后8字节为0    
	my_addr.sin_family = AF_INET;				// ipv4
	my_addr.sin_port   = htons(SERVERPORT);			// 端口
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);// ip，INADDR_ANY为通配地址其值为0
	socklen_t len = sizeof(my_addr); //地址结构长度

	/* 输入时，告诉bind函数，地址结构的长度，输出时实际的长度 绑定IP和端口号*/
	ret = bind(fd,(struct sockaddr *)&my_addr,len);
	if(ret < 0) {
		perror("bind error");
		return -1;
	}
	printf("server bind success!!\n");

	//设置监听数
	ret = listen(fd,10);
	if(ret < 0) {
		perror("listen error");
		return -1;
	}
	printf("start accept listen ... ... \n");
   
	struct sockaddr_in clientaddr;
	socklen_t clientaddr_len = sizeof(clientaddr);
	char cli_ip[INET_ADDRSTRLEN] = "";
	int connect_fd;
	while(1)
	{
        // 接受客户端请求
		connect_fd = accept(fd,
				(struct sockaddr*)&clientaddr,
				&clientaddr_len);
		if(connect_fd<0)
		{
			perror("accept error");
			continue;
		}
		break;
	}
	
	inet_ntop(AF_INET, &clientaddr.sin_addr, cli_ip, INET_ADDRSTRLEN);
	printf("----------------------------------------------\n");
	printf("client ip=%s,port=%d\n", cli_ip,ntohs(clientaddr.sin_port));
	
	while(1) {
		char buf[1000] = {0};
		int ret = recv(connect_fd,buf,sizeof(buf),0);
		if(ret < 0) {
			perror("recv error");
			return -1;
		}
		else if(ret == 0) {
			printf("no data\n");
		}
		else if(ret>0) {
			printf("recv data = %s\n",buf);
		}
		sleep(1);
	}

	return 0;
}
