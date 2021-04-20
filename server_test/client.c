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
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);// 创建通信端点：套接字
    if(sockfd < 0)
    {
        perror("socket");
        exit(-1);
    }

	 // 设置服务器地址结构体
     struct sockaddr_in server_addr;
     bzero(&server_addr,sizeof(server_addr)); // 初始化服务器地址
     server_addr.sin_family = AF_INET;   // IPv4
     server_addr.sin_port = htons(SERVERPORT);   // 端口
     inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);    // ip

      // 主动连接服务器
     int err_log = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
     if(err_log != 0)
     {
         perror("connect");
         close(sockfd);
         exit(-1);
     }
    
      while(1)
      {
		  char send_buf[512] = {0};
          printf("send:");
          strcpy(send_buf,"aaaaaa");
          send_buf[strlen(send_buf)-1]='\0';
          send(sockfd, send_buf, strlen(send_buf), 0);   // 向服务器发送信息
		  sleep(1);
      }

	  return 0;
}   

