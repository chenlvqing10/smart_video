#include <stdio.h>           
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>

#define  QUIT     "quit"
#define  BUF_SIZE 10000
#define  SERVER_PORT     8888
#define  SERVER_IP  "192.168.1.136"
#define  CLIENT_IP	"192.168.1.136"
#define  CLIENT_PORT  6667
void pipesig_handler(int signo)
{
	printf("捕获到了SIGPIPE信号\n");
}


int main()
{
	//创建套接字
	int serv_sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(serv_sock<0)
	{
		perror("socket create");
		exit(EXIT_FAILURE);
	}
	else
	{
		perror("socket create");
		printf("serv_sock=%d\n",serv_sock);
	}
	//地址快速重用
	int b_reuse = 1;
	if(setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int)) <0){
		perror("setsockopt");
		exit(1);
	}
	puts("----设置地址快速重用成功----");

	//设置服务器端 IP，端口号
	struct sockaddr_in serv_addr;//服务器端地址
	memset(&serv_addr,0,sizeof(serv_addr));//地址大小
	serv_addr.sin_family=AF_INET;//IP地址类型:ipv4
	serv_addr.sin_addr.s_addr=inet_addr(SERVER_IP);//服务器地址
	serv_addr.sin_port=htons(SERVER_PORT);//端口号//服务器端口号
	
	//设置客户端IP地址和端口号
	struct sockaddr_in clnt_addr;//客户端地址
	memset(&clnt_addr,0,sizeof(clnt_addr));//地址大小
	clnt_addr.sin_family=AF_INET;//IP地址类型:ipv4
	clnt_addr.sin_addr.s_addr=inet_addr(CLIENT_IP);//客户端地址
	clnt_addr.sin_port=htons(CLIENT_PORT);//客户端端口号

	//绑定客户端
	if(bind(serv_sock,(struct sockaddr*)&clnt_addr,sizeof(clnt_addr))<0)//将套接字与特定的IP地址和端口号绑定起来
	{
		perror("bind server");
		exit(EXIT_FAILURE);
	}
	else
	{
		perror("bind server");
		printf("客户端就绪\n");
	}

	if(connect(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)//客户端与服务器端进行连接  TCP三次握手连接
	{
		perror("connect server");//连接失败
		exit(EXIT_FAILURE);
	}
	else
		perror("connect server");//连接成功

	char buffersend[BUF_SIZE]={0};//客户端发送缓冲区
	//创建集合，初始化集合
	fd_set rfds;//文件描述符集合
	int		maxfd;//最大的文件描述符
	//设置超时时间
	struct timeval tv={1,0};
	tv.tv_sec=1;
	tv.tv_usec=0;
	int timecount = 0;

    int retval,ret;
	
	while(1)
	{
		signal(SIGPIPE, pipesig_handler);//添加信号处理函数
        FD_ZERO(&rfds);//清空fd集合
		FD_SET(0, &rfds);//向fd集合中添加文件描述符(stdin)
		FD_SET(serv_sock,&rfds);//将套接字加入到fd集合中
		maxfd=serv_sock;

		retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
		if (retval == -1)//select error
		{
			perror("select()");
			exit(EXIT_FAILURE);
		}
		else if(retval == 0)//timeout
		{
			printf("timeout within 1 seconds.\n");
			timecount+=1000;
			sleep(1);
			if(timecount>=5000) {
				printf("send a data\n");
				timecount = 0;
				write(serv_sock,"i am lucy!",strlen("i am lucy!"));
			}
			continue;
		}
		else if (retval>0)//data is available
		{
			printf("有数据\n");
			if(FD_ISSET(0,&rfds))
			{//键盘输入了
				printf("有标准输入事件发生\n");
				bzero(buffersend,BUF_SIZE);
				while(((ret=read(0,buffersend,BUFSIZ-1))<0)&&(EINTR==errno))//读取键盘输入的数据到缓冲区
				{
					perror("read");
					continue;
				}

				if(ret>0)
					printf("read stdin data success\n");

				if(write(serv_sock,buffersend,strlen(buffersend))<0)//将缓冲区的数据写入套接字发给服务端
				{
					perror("write server socket");
					exit(1);
				}
				else
				{
					printf("write server sockete success\n");
					printf("写入套接字的内容:%s\n",buffersend);
				}

				if(!strncasecmp(buffersend,QUIT,strlen(QUIT)))
				{
					printf("***客户端退出***\n");
					break;
				}
			}


			if(FD_ISSET(serv_sock,&rfds))//服务端发送数据的时候接收数据
			{
				printf("服务端发送了数据\n");
				bzero(buffersend,BUFSIZ);
				ret=read(serv_sock,buffersend,BUFSIZ-1);//读取服务器发送过来的数据

				while((ret<0)&&(EINTR==errno))
				{
					perror("read");
					continue;
				}

				if(ret==0)
				{
					perror("read 0");
					printf("read 0 character\n");
					break;
				}
				else if(ret>0)//读取成功
				{
					printf("已收到：%s \n",buffersend);
					if(!strncasecmp(buffersend,QUIT,strlen(QUIT)))
					{
						printf("***因为服务器断开了连接，所以客户端退出***\n");
						break;
					}
				}
			}
		}

	}//end while
	printf("有没有执行到这里\n");
	close(serv_sock);
	printf("111111111111111111111111111111\n");
	return 0;
}
