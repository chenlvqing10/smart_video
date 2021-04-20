#ifndef __PLAYER__
#define __PLAYER__

 #include <sys/types.h>
#define SHMKEY  5550
#define SHMSIZE 40960


#define MUSICPATH "/home/chenlvqing/chenlvqing/smart_speaker/music/"
//#define MUSICPATH "/home/chenlvqing/chenlq/smart_speaker/music/"
#define SEQUENCEMODE 1  //顺序播放
#define RANDOMMODE   2  //随机播放
#define CIRCLE       3  //循环播放

/* 共享内存数据 */
struct shm{
	int play_mode;      //播放模式
	char cur_name[64];  //当前歌曲名

	pid_t ppid;         //父进程号
	pid_t child_pid;    //子进程号
	pid_t grand_pid;    //孙进程号
};

int  Initshm();
void m_select();
void GetMusic();
void start_play();
void stop_play();
void suspend_play();
void continue_play();
void prior_play();
void next_play();
void voice_up();
void voice_down();
void set_mode(int play_mode);

void socket_cmd_control(const char* cmd);



#endif
