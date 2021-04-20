#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "device.h"
#include "link.h"
#include "socket.h"
#include "player.h"
#include "select.h"
 //LinkList head node 
 struct Node* head;

int g_button_fd = 5;//button file
int g_led_fd;//led file
int g_mixer_fd;//mixer file
int g_socket_fd;//socket file

int g_maxfd;

int main()
{
	int ret;

	/*
	ret = initDriver();//open a device
	if(FAILURE == ret)
	{
		printf("init device file fail!!\n");
		exit(1);
	}
	*/

	//init network
	ret = InitSocket();
	if(FAILURE == ret)
	{
		//init fail the led0/led1 on
		led_on(0);
		led_on(1);
	}
	printf("init socket success\n");

	
	//init NodeList 创建一个空节点 双向循环列表
	ret = InitLink();
	if(ret == FAILURE)
	{
		printf("init link fail\n");
		exit(1);
	}
	printf("init link success\n");

	/* 初始化共享内存 */
	ret = Initshm();
	if(ret == FAILURE) {
		printf("init share memory fail\n");
		exit(1);
	}
	printf("init share memory success\n");

	/* 从文件目录下读取所有的音乐文件 read music 插入文件夹下所有歌曲到  一个分类齐全的歌曲数据库 */
	GetMusic();
	
	//listen the user operation
	m_select();

	return 0;
}
