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

long getTime_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    long time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    
    printf("second: %ld\n", tv.tv_sec); // 秒
    printf("millisecond: %ld\n", tv.tv_sec * 1000 + tv.tv_usec / 1000); // 毫秒
    printf("microsecond: %ld\n", tv.tv_sec * 1000000 + tv.tv_usec); // 徽秒

    return time_ms;
}

typedef int (*CallBackFun)(int fd, char *buf, unsigned int bufLen);

struct connect_info{
    long time_ms;
};

struct conncet_message{
    pthread_mutex_t con_lock;
    int fd;
    int connect_flag;
    struct connect_info info;
    CallBackFun action;  
};

struct conncet_message connect_message;

 int client_action(int fd, char *buf, unsigned int bufLen)
 {
     printf("call back buffer is %s\n",buf);
	 return 0;
 }

void* connectToServer(void* arg)
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
        connect_message.fd = serv_sock;
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
		close(serv_sock);
		return (void*)"failed";
	}
	else {
        connect_message.connect_flag = 1;
        //得到时间戳
        connect_message.info.time_ms = getTime_ms();
        //调用回调函数
        connect_message.action(serv_sock,"i am callback function",strlen("i am callback function"));

        printf("connect server\n");//连接成功
		return (void*)"ok";
    }
}

int main()
{
    int ret;
    pthread_mutex_init(&connect_message.con_lock,NULL);
    connect_message.action = &client_action;

    pthread_t thread1,thread2;
    ret = pthread_create(&thread1,NULL,connectToServer,NULL);
    //pthread_join(thread1,NULL);


    while(1) {
        printf("hello\n");
		if(connect_message.connect_flag == 0)
			connectToServer(NULL);
        //sleep(1);
    }
    
   pthread_join(thread1,NULL);

    return 0;
}