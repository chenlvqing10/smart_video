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
#include "player.h"
#include <json-c/json.h>
#define  BUF_SIZE 10000
extern int g_button_fd;//button file
extern int g_socket_fd;//socket file
extern int g_maxfd;

/* for test */
void show()
{
	printf("1.start    play\n");
	printf("2.stop     play\n");
	printf("3.pause    play\n");
	printf("4.continue play\n");
	printf("5.last     music\n");
	printf("6.next     music\n");
	printf("7.Up       Volume\n");
	printf("8.Down     Volume\n");
	printf("9.设置顺序播放模式\n");
	printf("a.设置随机播放模式\n");
	printf("b.设置循环播放模式\n");
}
void send_server(void)//信号处理函数
{
	//向服务器发送一个JSON包
	printf("send data to server\n");
	struct json_object* json = json_object_new_object();
	json_object_object_add(json,"status",json_object_new_string("alive"));
	
	const char* buf = json_object_to_json_string(json);
	printf("buf = %s\n",buf);
	int ret = write(g_socket_fd,buf,strlen(buf));
	if(-1 == ret) {
		perror("send");
		exit(-1);
	}
	printf("send success\n");
}
void decode_server_cmd(const char* bufRecv,char* cmd)
{
	if(bufRecv == NULL)
		printf("NULL\n");
	printf("call decode_server_cmd\n");
	struct json_object*obj = json_tokener_parse(bufRecv);
	struct json_object* decode_json;
	json_object_object_get_ex(obj,"cmd",&decode_json);
	strcpy(cmd,json_object_get_string(decode_json));
}


void m_select()
{
	show();
	//define fd and to zero
	fd_set readfd;
	int ret;
	int timecount = 0;
	struct timeval tv={0,0};
	while(1)
	{
		//printf("select loop\n");
		FD_ZERO(&readfd);
		//add button/socket file to fd
		//FD_SET(g_button_fd,&readfd);
		FD_SET(0,&readfd);  /* 添加标准输入到文件描述符集合中 */
		FD_SET(g_socket_fd,&readfd);  /* 添加套接字到文件描述符集合中 */
		g_maxfd = g_socket_fd;

		int retval = select(g_maxfd+1, &readfd, NULL, NULL, &tv);
		if (retval == -1)//select error
		{
			perror("select()");
			exit(EXIT_FAILURE);
		}
		else if(retval==0)//timeout
		{
			/*printf("timeout within 1 seconds.\n");
			timecount+=1000;
			sleep(1);
			if(timecount>=5000) {
				printf("send a data\n");
				timecount = 0;
				send_server();
			}
			*/
			continue;
		}
		else if (retval>0)//data is available
		{
			printf("有数据\n");
			char buffersend[BUF_SIZE] ;

			if(FD_ISSET(0,&readfd))
			{//键盘输入了
				printf("有标准输入事件发生\n");
				char func;
				scanf("%c",&func);
				switch(func)
				{
					case '1': // 开始播放 
						printf("开始播放音乐\n");
						start_play();
						break;
					case '2': // 停止播放 
						printf("停止播放音乐\n");
						stop_play();
						break;
					case '3': // 暂停播放 
						printf("暂停播放音乐\n");
						suspend_play();
						break;
					case '4': // 继续播放 
						printf("继续播放音乐\n");
						continue_play();
						break;
					case '5': // 上一首
						printf("上一首\n");
						prior_play();
						break;
					case '6': // 下一首 
						printf("下一首\n");
						next_play();
						break;
					case '7': // 增加音量 
						printf("增加音量\n");
						voice_up();
						break;
					case '8': // 减少音量
						printf("减少音量\n");
						voice_down();
						break;
					case '9':
						printf("顺序播放模式\n");
						set_mode(SEQUENCEMODE);
						break;
					case 'a':
						printf("随机播放模式\n");
						set_mode(RANDOMMODE);
						break;
					case 'b':
						printf("循环播放模式\n");
						set_mode(CIRCLE);
						break;
					default:
						break;
				}

			}//end std input

			printf("11111111111\n");


			if(FD_ISSET(g_socket_fd,&readfd))//服务端发送数据的时候接收数据
			{
				printf("服务端发送了数据\n");
				char bufRecv[1000] = {0};
				bzero(bufRecv,BUFSIZ);

				ret=recv(g_socket_fd,bufRecv,sizeof(bufRecv),0);//读取服务器发送过来的数据
				while((ret < 0) && (EINTR == errno))
				{
					printf("recv error\n");
					continue;
				}

				if(ret == 0)
				{
					printf("read 0 character\n");
					break;
				}
				else if(ret > 0)//读取成功
				{
					printf("已收到：%s \n",bufRecv);

					//解析服务器过来的命令
					char cmd[100] = {0};
					decode_server_cmd(bufRecv,cmd);

				    socket_cmd_control(cmd);
					
				
					


					
				
				}
			}//end from server
		}
	
		
	}

}
