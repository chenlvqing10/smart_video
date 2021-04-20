#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "link.h"
#include "player.h"
#include <time.h>
extern struct Node* head;

/* 初始化双向循环链表 */
int InitLink()
{
	head = (Node*) malloc(sizeof(Node)*1);
	if(NULL == head)
	{
		return FAILURE;
	}

	head->next  = head;
	head->prior = head;
	strcpy(head->music_name,"");
	
	return SUCCESS;
}

/* 尾插法插入到链表中 */
int InsertLink(Node* head,const char* name)
{
	if(( NULL == head ) || ( NULL == name ))
		return FAILURE;

	Node* end = head->prior; /* 当前链表的尾结点 */

	/* 创建一个新的节点 */
	struct Node* newNode = (struct Node*) malloc(sizeof(Node) * 1);
	if( NULL == newNode ) {
		return FAILURE;
	}

	newNode->next = head;
	strcpy(newNode->music_name,name);
	end->next = newNode;
	newNode->prior = end;
	head->prior = newNode;
	
	end = NULL;
	free(end);

	return SUCCESS;
}

/* 函数描述：根据当前歌曲和播放模式，找到下一首歌曲
 * 函数参数：
 * cur:当前歌曲
 * mode:播放模式
 * next:下一首歌曲名
 * */
void FindNextMusic(const char* cur_name,int mode,char* next_name)
{
	printf("call %s\n",__FUNCTION__);
	printf("head->next.music_name = %s cur_name = %s \n",head->next->music_name,cur_name);
	
	if(mode == CIRCLE) {
		printf("循环播放\n");
		strcpy(next_name,cur_name);
	}

	if(mode == SEQUENCEMODE) {
		printf("顺序播放\n");
		
		/* p指向当前节点 */
		Node* p = head->next;
		while(strcmp(p->music_name,cur_name) != 0) {
			p = p->next;
		}
		strcpy(next_name,p->next->music_name);
		return;
	}

	if(mode == RANDOMMODE) {
		printf("随机播放\n");
		Node* p = head->next;
		srand(time(NULL));
		int num = rand() % 100;
		int i;
		for(i=0;i<num;i++) {
			p = p->next;
		}
		strcpy(next_name,p->music_name);
	}

	printf("下一首歌曲是%s\n",next_name);
	return;
}

/* 跳转到上一首歌曲 */
void ToPriorMusic(const char* cur_name,int mode,char* prior_name)
{
	printf("call %s\n",__FUNCTION__);
	printf("head->next.music_name = %s cur_name = %s \n",head->next->music_name,cur_name);

	if(mode == SEQUENCEMODE || mode == CIRCLE) {//顺序播放 循环播放
		printf("顺序播放 或者循环播放\n");
		
		/* p指向当前节点 */
		Node* p = head->next;
		while(strcmp(p->music_name,cur_name) != 0) {
			p = p->next;
		}

		/* 如果是头结点 */
		if(p->prior == head) {
			printf("头结点 上一首的歌曲名是%s\n",p->prior->prior->music_name);
			strcpy(prior_name,p->prior->prior->music_name);
		}
		else
		{
			printf("非头结点 上一首的歌曲名是%s\n",p->prior->music_name);
			strcpy(prior_name,p->prior->music_name);
		}
		
		return;
	}

	if(mode == RANDOMMODE) {//随机播放
		Node* p = head->next;
		srand(time(NULL));
		int num = rand() % 100;
		int i;
		do {
			for(i=0;i<num;i++)
				p = p->next;
		}while(p->prior==head);

		strcpy(prior_name,p->prior->music_name);
	}
	return;
}

/* 跳转到下一首歌曲 */
void ToNextMusic(const char* cur_name,int mode,char* next_name)
{
	printf("call %s\n",__FUNCTION__);
	printf("head->next.music_name = %s cur_name = %s \n",head->next->music_name,cur_name);

	if(mode == SEQUENCEMODE || mode == CIRCLE) {//顺序播放 循环播放
		printf("顺序播放 或者循环播放\n");

		/* p指向当前节点 */
		Node* p = head->next;
		while(strcmp(p->music_name,cur_name) != 0) {
			p = p->next;
		}

		/* 如果当前节点是尾结点 */
		if(p->next == head) {
			printf("尾结点 下一首的歌曲名是%s\n",p->next->next->music_name);
			strcpy(next_name,p->next->next->music_name);
		}
		else
		{
			printf("非尾结点 上一首的歌曲名是%s\n",p->prior->music_name);
			strcpy(next_name,p->next->music_name);
		}		
		return;
	}

	if(mode == RANDOMMODE) {//随机播放
		Node* p = head->next;
		srand(time(NULL));
		int num = rand() % 100;
		int i;
		do {
			for(i=0;i<num;i++)
				p = p->next;
		}while(p->next==head);
	}
	return;

}