#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define  QUIT     		"quit"
#define  BUF_SIZE 		10000
#define  SERVER_PORT    8888
#define  SERVER_IP  	"192.168.1.136"                       

void pipesig_handler(int signo)
{
	printf("捕获到了SIGPIPE信号\n");  
	printf("客户端未连接或者套接字有问题\n");
}

void get_sigsegv(int signo)
{
	printf("捕获到了SIGSEGV信号\n");  
	printf("客户端未连接或者套接字有问题\n");
	exit(-1);
}


int main()
{
	char buffer[BUF_SIZE]={0};//定义缓冲区

	//服务器端:创建TCP套接字
	int serv_sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(serv_sock < 0)
	{
		perror("socket create");
		exit(EXIT_FAILURE);
	}
	printf("服务器端套接字创建成功\n");
	
	//地址快速重用
	int b_reuse = 1;
	if(setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int)) <0){
		perror("setsockopt");
		exit(1);
	}
	puts("设置地址快速重用成功\n");

	//设置服务器的IP,端口号
	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));//空间清0
	serv_addr.sin_family=AF_INET;//使用ipv4地址
	serv_addr.sin_addr.s_addr=htons(INADDR_ANY);//设置服务器端IP
	serv_addr.sin_port=htons(SERVER_PORT);//端口号

	//将套接字与特定的IP地址和端口号绑定起来
	if(bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
	{
		perror("bind server");
		exit(EXIT_FAILURE);
	}
	printf("绑定成功,服务器就绪\n");

	//服务端 进入监听状态，等待客户端用户发起请求   TCP
	if(listen(serv_sock,10)<0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("服务器监听就绪.......\n");

	//定义客户端地址的结构体
	struct sockaddr_in client_addr;//客户端地址  保存了客户端的IP地址和端口号
	socklen_t client_addr_size=sizeof(client_addr);//客户端地址的长度

	//创建文件描述符集合，初始化集合
	fd_set rfds,wfds,efds;//文件描述符集合  可读文件描述符集合 可写文件描述符集合 错误异常文件描述符集合
	fd_set temp;//临时文件描述符集合
	int    maxfd;//最大的文件描述符

	//select超时时间 ：
	//1.若将NULL以形参传入，即不传入时间结构，就是将select置于阻塞状态，一定等到监视文件描述符集合中某个文件描述符发生变化为止
	//2.若将时间值设为0秒0毫秒，就变成一个纯粹的非阻塞函数，不管文件描述符是否有变化，都立刻返回继续执行，文件无变化返回0，有变化返回一个正值
	//3.timeout的值大于0，这就是等待的超时时间，即 select在timeout时间内阻塞，超时时间之内有事件到来就返回了，否则在超时后不管怎样一定返回，返回值同上述。
	//设置超时时间
	//struct timeval tv={10,0};
	//tv.tv_sec=10;
	//tv.tv_usec=0;

	int retval,ret;
	int newsock;
	maxfd= serv_sock;//将服务端的套接字赋值给maxfd  初始化
	FD_ZERO(&rfds);//清空fd集合
	FD_SET(0, &rfds);//向fd集合中添加文件描述符(stdin)
	FD_SET(serv_sock,&rfds);//将套接字加入到fd集合中
	printf("如果要向客户端发送数据  请按以下格式进行标准输入的处理\n");
	printf("客户端套接字的号码 输入的内容\n");

	//I0多路复用
	while(1)
	{
		//printf("server loop\n");
		
		//信号处理函数
        signal(SIGPIPE, pipesig_handler);
		signal(SIGSEGV,get_sigsegv);

		//select函数 监听客户端时间 标准输入事件
		temp=rfds;//可读文件描述符集合数组赋值给temp    temp保存了所有新连接的文件描述符
		retval = select(maxfd+1, &temp, NULL, NULL, 0);
		if (retval == -1)//select error
		{
			perror("select error");
			if(errno == EINTR)
				continue;
			exit(EXIT_FAILURE);
		}

		if(retval==0)//timeout
		{
			//printf("timeout within 5 seconds.\n");
			//sleep(2);
			continue;
		}

		if (retval>0)//data is available 有事件响应
		{   printf("有事件响应...................................................\n");
			printf("retval=%d  maxfd=%d \n",retval,maxfd);

			//轮训判断那个文件描述符产生事件
			for(int i=0;i<maxfd+1;i++)//fd=0 1 2 3 4 ...
			{
				if(FD_ISSET(i,&temp))//如何知道是那个事件响应？
				{
					printf("event fd %d is true and maxfd = %d\n",i,maxfd);
					if(i==0)//是键盘输入产生事件  服务器发数据给客户端
					{
						printf("标准输入事件\n");
						int input_t,input_fd;
						bzero(buffer,sizeof(buffer));
						input_t = scanf("%d %s",&input_fd,buffer);
						while(getchar()!='\n');

						if((input_t !=2) || (!strcmp(buffer,""))){
							printf("输入错误！请按格式重新输入：fd+空格+消息内容\n");
							continue;
						}

						if(!FD_ISSET(input_fd,&rfds)){
							printf("输入fd号错误，没有该客户端套接字，请重新输入\n");
							continue;
						}
						
						//向客户端发出信息 最多发送5次
						printf("send data to client\n");
						int count =0;
						do{
							ret = write(input_fd,buffer,strlen(buffer));
							if(ret > 0) {
								printf("write success\n");
								break;
							}
							count++;
							if(count >=5 ) {
								printf("send failed\n");
								break;
							}
						}while(ret < 0 && EINTR == errno);//如果发生错误继续发送一直到成功为止
					}
					
					//如果是socket产生的套接字文件描述符  那就说明是要建立新的客户端连接
					if(i==serv_sock)
					{
						//产生一个新生成的套接字 接受客户端的请求时候激活accept函数 
						newsock=accept(serv_sock,(struct sockaddr*)&client_addr,&client_addr_size);
						if(newsock==-1)//接收失败
						{
							perror("accept client");
							close(newsock);
							exit(EXIT_FAILURE);
						}
						else//接收成功
						{
							perror("accept client");
						}

						//打印客户端信息
						char ip[20]={0};
						if((inet_ntop(AF_INET, (void *)&client_addr.sin_addr.s_addr,ip,sizeof(client_addr)))==NULL)//将IP转化为点分十进制形式
						{
							perror("ip error");
							exit(EXIT_FAILURE);
						}
						else
						{
							printf("++++++客户端++++++  ip=%s   port=%d\n",ip,ntohs(client_addr.sin_port));
						}

						//将产生的新套接字加入文件描述符的集合之中,更新maxfd(判断那个最大的文件描述符)
						FD_SET(newsock,&rfds);
						maxfd = newsock > maxfd ? newsock : maxfd;
						printf("newsock=%d maxfd=%d\n",newsock,maxfd);//打印新的套接字和最大的文件描述符
					}
					else if(i > serv_sock)//处理已经连接好的客户端请求和送来的数据
					{
						//信号处理函数
						signal(SIGPIPE, pipesig_handler);
						signal(SIGSEGV,get_sigsegv);

						//i就是产生事件的客户端对应的socket  从客户端接收消息
						printf("客户端%d发送了数据过来\n",i);
						bzero(buffer,BUFSIZ);

						while(((ret = read(i,buffer,BUFSIZ)) <0 ) && (EINTR == errno))//读取新的fd的数据到缓冲区
						{
							perror("read ret < 0");
							continue;
						}

						if(ret > 0)//读取成功
						{
							perror("read");
							printf("从客户端%d收到数据：%s \n",i,buffer);
						}
						else if(ret == 0)//客户端关闭
						{
							printf("no read data from client\n");
							printf("buffer：%s\n",buffer);
							printf("***客户端%d退出***\n",i);
							FD_CLR(i,&rfds);//将该客户端套接字从文件描述符集合中清除中
							close(i);//关闭客户端
							maxfd = (( i == maxfd) ? (--maxfd) : maxfd);//如果关闭的是最大的文件描述符则将文件描述符-1
						}
					}
				}
			}//end 轮询套接字
		}//end 事件响应
	}//end while

	printf("是否执行到了这里\n");
	close(serv_sock);

	return 0;
}
