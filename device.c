//use to operate the hardware device
#include "main.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <stdio.h>

extern int g_button_fd;//button file
extern int g_led_fd;//led file
extern int g_mixer_fd;//mixer file         


int initDriver()
{
	//open a button device file
/*	g_button_fd = open("/dev/buttons",O_RDONLY);
	if(-1 == g_button_fd)
	{
		return FAILURE;
	}

*/
	//open leds  device file
/*	g_led_fd = open("/dev/leds",O_WRONLY);
	if(-1 == g_led_fd)
	{
		return FAILURE;
	}

	//ALL LEDS off
	for(int i=0;i<4;i++)
	{
		ioctl(g_led_fd,0,i);	
	}
*/

	//open mixer device file
	g_mixer_fd = open("/dev/mixer",O_WRONLY);
	if(-1 == g_mixer_fd)
	{
		return FAILURE;
	}

	int vol;  
	ioctl(g_mixer_fd, MIXER_READ(SOUND_MIXER_MIC), &vol);
	printf("Mic gain is at %d %%\n", vol); 

	int left, right;
	left = vol & 0xff;
	right = (vol & 0xff00) >> 8;
	printf("Left gain is %d %%, Right gain is %d %%\n", left, right); 

	return SUCCESS;
}


void led_on(int which)
{
	ioctl(g_led_fd,1,which);
}

void led_off(int which)
{
	ioctl(g_led_fd,0,which);
}
