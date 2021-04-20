#include <stdio.h>
#include "link.h"
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "main.h"
#include "player.h"
#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <json-c/json.h>
#include <json-c/json_object.h>

extern Node* head;
char* g_addr = NULL; /* 共享内存映射地址 */
int g_start_flag = 0; //开始播放
int g_suspend_flag = 0; //暂停播放
extern int g_mixer_fd;//mixer file
extern int g_button_fd;//button file
extern int g_socket_fd;//socket file

int Initshm()
{
	//创建共享内存
	int shmid = shmget(SHMKEY, SHMSIZE, IPC_CREAT | IPC_EXCL | 0666);
	if ( -1 == shmid)
	{
		printf("error init shm\n");
		return FAILURE;
	}
 
	//映射
	g_addr = (char*)shmat(shmid,NULL,0);
	if( (NULL == g_addr) || (-1 == *(int*)g_addr) ) {
		perror("shmat");
		return FAILURE;
	}
		
	printf("map success\n");
	printf("g_addr = %p\n",g_addr);

	//初始化共享内存数据
	struct shm s;
	s.play_mode = SEQUENCEMODE;
	s.ppid = getppid();

	//printf("sizeof s = %ld\n",sizeof(s));
	memcpy(g_addr, &s, sizeof(s));

	return SUCCESS;
}


int m_mp3_end(const char* name)
{
	const char* ptr = name;

	/* 指针移动文件名的末尾 */
	while(*ptr != '\0') {
		ptr ++ ;
	}

	/* 指针移动到.的位置 */
	for(int i =0;i<4;i++) {
		ptr--;
	}

	return (strcmp(ptr,".mp3") == 0 ? 1 : 0);

}

void GetMusic()
{
	//open music dir
	DIR *dir = opendir(MUSICPATH);
	if(NULL == dir) {
		perror("opendir");
		exit(1);
	}

	/* 读取目录 */
	struct dirent* file;
	while (NULL != (file = readdir(dir))) {
		/* 判断是否是普通的文件 */
		if(file->d_type != 8) {
			continue;
		}

		/* 判断是否是MP3文件 */
		if(!m_mp3_end(file->d_name)) {
			continue;	
		}
		printf("歌曲的名字:%s\n",file->d_name);
		/* 将歌曲放入到链表中 */
		int ret = InsertLink(head,file->d_name);
		if(FAILURE == ret) {
			printf("歌曲插入失败\n");
			exit(-1);
		}
	}
}

void play_music(const char* name)
{
	printf("call %s\n",__FUNCTION__);

	pid_t child_pid = fork();
	char temp_name[100] = {0};
	strcpy(temp_name,name);

	if(-1 == child_pid) {
		perror("fork");
		exit(1);
	}
	else if(0 == child_pid) {  /* 子进程 */
		while(1) {
			pid_t grand_pid = vfork();
			if(-1 == grand_pid) {
				perror("frok");
				exit(1);
			}
			else if(0 == grand_pid) { /* 孙进程 */
				char cur_name[64] = {0};
				struct shm s;

				//获取共享内存
				int shmid = shmget(SHMKEY,SHMSIZE,0);
				if(-1 == shmid) {
					perror("shmget");
					exit(1);
				}
				//映射
				void* addr = shmat(shmid,NULL,0);
				if(NULL == addr) {
					perror("shmat");
					exit(1);
				}
				memcpy(&s,addr,sizeof(s));//读取共享内存中的数据到结构体

				if(strlen(temp_name) != 0) {
					printf("有歌曲名，直接开始播放！！\n");
					/* 直接开始播放 */
					strcpy(cur_name,temp_name);
				}
				else {
					printf("孙进程歌曲播放完了，清空歌曲名之后，需要遍历链表\n");
					/* 遍历链表 找到一首歌进行播放 */
					//判断播放模式，找到一首歌曲
					printf("当前播放完了的歌曲名是%s 播放模式是%d head->next.music_name = %s\n",s.cur_name,s.play_mode,head->next->music_name);
					FindNextMusic(s.cur_name,s.play_mode,cur_name);
				}

				/* 把信息写入共享内存 父子孙进程ID 当前歌曲名*/
				printf("播放模式:%d\n",s.play_mode);
				strcpy(s.cur_name,cur_name);
				s.child_pid = getppid();
				s.grand_pid = getpid();
				memcpy((char*)addr,&s,sizeof(s));
				shmdt(addr);/* 解除映射 */

				printf("子进程进程id:%d ,孙进程进程id:%d\n",s.child_pid,s.grand_pid);
				
				/* 播放歌曲 */
				char music_path[128] = {0}; //包含路径的歌曲名称
				strcpy(music_path,MUSICPATH);
				strcat(music_path,cur_name);
				printf("歌曲名字 %s\n", music_path);
				execl("/usr/bin/madplay","madplay",music_path,NULL);
			}
			else { /* 子进程 */
				printf("孙进程歌曲播放完了！\n");
				printf("head->next.music_name = %s\n",head->next->music_name);
				memset((char*)temp_name,0,strlen(temp_name));  //歌曲名长度变为0,方便下一次操作
				int status;
				waitpid(grand_pid,&status,0);  /* 回收孙进程 */
			}
		}

	}
	else {
		printf("父进程不做说明事情\n");
		return;
	}

}


/* 开始播放音乐 */
void start_play()
{
	if(g_suspend_flag == 1) {//按了暂停键
		continue_play();
		return;
	}

	if(g_start_flag == 1 ) { //已经开始播放
		return;
	}	

	//获取歌曲名称
	if(head->next == NULL) { //空链表，没有歌曲
		return;
	}

	//设置音量
	

	//开始播放音乐   播放哪一首歌曲 由用户点击决定  用户没有点击，则从第一首开始
	play_music(head->next->music_name);//传入链表中的歌曲名字  从第一首开始播放

	g_start_flag = 1;
	g_suspend_flag = 0;
}

void stop_play()
{
	if(g_start_flag == 0)
		return;

	//读取共享内存 
	struct shm s;
	memset(&s,0,sizeof(s));
	memcpy(&s, g_addr, sizeof(s));

	kill(s.grand_pid, SIGKILL);    //结束子进程
	kill(s.child_pid, SIGKILL);    //结束孙进程

	g_start_flag = 0;
    g_suspend_flag =0;
}

void suspend_play()
{
	if(g_start_flag == 0 || g_suspend_flag == 1) {
		return;
	}

	//读取共享内存 
	struct shm s;
	memset(&s,0,sizeof(s));
	memcpy(&s, g_addr, sizeof(s));

	//暂停孙子进程
	kill(s.grand_pid,SIGSTOP);//暂停孙进程
	kill(s.child_pid,SIGSTOP);//暂停子进程

	//更新标志位
	g_suspend_flag = 1;
}

void continue_play()
{
	if(g_start_flag == 0 || g_suspend_flag == 0) {
		return;
	}

	//读取共享内存 
	struct shm s;
	memset(&s,0,sizeof(s));
	memcpy(&s, g_addr, sizeof(s));

	//继续孙子进程
	kill(s.grand_pid,SIGCONT);//继续孙进程
	kill(s.child_pid,SIGCONT);//继续子进程

	//更新标志位
	g_suspend_flag = 0;

}

void prior_play()
{
	if(g_start_flag == 0) {
		return;
	}

	//读取共享内存 
	struct shm s;
	memset(&s,0,sizeof(s));
	memcpy(&s, g_addr, sizeof(s));

	kill(s.grand_pid, SIGKILL);    //结束子进程
	kill(s.child_pid, SIGKILL);    //结束孙进程

	g_start_flag =0;

	printf("stop music success\n");
	
	//找到上一首歌
	char prior_music_name[64];
	printf("当前歌曲的名字是%s 播放模式是%d\n",s.cur_name,s.play_mode);
	ToPriorMusic(s.cur_name,s.play_mode,prior_music_name);
	play_music(prior_music_name);
	printf("上一首歌曲是%s\n",prior_music_name);
	g_start_flag = 1;

}

void next_play()
{
	if(g_start_flag == 0) {
		return;
	}

	//读取共享内存 
	struct shm s;
	memset(&s,0,sizeof(s));
	memcpy(&s, g_addr, sizeof(s));

	kill(s.grand_pid, SIGKILL);    //结束子进程
	kill(s.child_pid, SIGKILL);    //结束孙进程

	//找到下一首歌曲
	char next_music_name[64];
	printf("当前歌曲的名字是%s 播放模式是%d\n",s.cur_name,s.play_mode);
	ToNextMusic(s.cur_name,s.play_mode,next_music_name);
	printf("下一首歌曲是%s\n",next_music_name);
	play_music(next_music_name);

}
#define VOLUME_MIN 0
#define VOLUME_MAX 100
static int ileft  = 20;
static int iright = 60;
void voice_up()
{
	int ilevel = 0;
	if (ileft < VOLUME_MAX)
	{
		ileft += 5;
	}

	ilevel = (iright << 8) + ileft;
	ioctl(g_mixer_fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &ilevel);//用来设置麦克风的输入增益

}


void voice_down()
{
	int ilevel = 0;

	if (ileft > VOLUME_MIN)
	{
		ileft -= 5;
	}

	ilevel = (iright << 8) + ileft;
	ioctl(g_mixer_fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &ilevel);//用来设置麦克风的输入增益

}

void set_mode(int play_mode)
{
	//读取共享内存 
	struct shm s;
	memset(&s,0,sizeof(s));
	memcpy(&s, g_addr, sizeof(s));

	s.play_mode = play_mode;

	//更新共享内存
	memcpy((char*)g_addr,&s,sizeof(s));
	printf("当前播放模式是%d\n",s.play_mode);
}

void socket_cmd_control(const char* cmd)
{
	//判断音箱的状态
    
	//根据服务器不同的命令执行不同的操作

	if(!strcmp(cmd,"start")) {
		start_play();
	}

	//结束播放
	if(!strcmp(cmd,"stop")) {
		stop_play();
	}

	//暂停播放
	if(!strcmp(cmd,"suspend")) {
		suspend_play();
	}

	//继续播放
	if(!strcmp(cmd,"continue")) {	
		continue_play();
	}
	
	//上一首
	if(!strcmp(cmd,"prior")) {
		prior_play();
	}

	//下一首
	if(!strcmp(cmd,"next")) {
		next_play();
	}					
	
	//增加音量
	if(!strcmp(cmd,"voice_up")) {
		voice_up();
	}

	//减少音量									
	if(!strcmp(cmd,"voice_down")) {
		voice_down();
	}

	//顺序播放模式					
	if(!strcmp(cmd,"sequence")) {
		set_mode(SEQUENCEMODE);
	}

	//随机播放模式
	if(!strcmp(cmd,"random")) {
		set_mode(RANDOMMODE);	
	}

	//循环播放模式
	if(!strcmp(cmd,"circle")) {
		set_mode(CIRCLE);
	}

	//获取音箱的状态
	if(!strcmp(cmd,"get")) {
		set_mode(CIRCLE);
	}

	//获取音箱中的所有音乐

	//回复服务器开始播放的结果
	struct json_object* json = json_object_new_object();
	json_object_object_add(json,"result",json_object_new_string("success"));

	const char* buf = json_object_to_json_string(json);
	printf("buf = %s\n",buf);
	int ret = write(g_socket_fd,buf,strlen(buf));
	if(-1 == ret) {
		perror("send");
		exit(-1);
	}
	printf("send success\n");

}