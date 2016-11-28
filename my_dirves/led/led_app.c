/*************************************************************************
    > File Name: led_app.c
    > Author: dooon
    > Mail: tangxu314@163.com 
    > Created Time: 2016年11月10日 星期四 21时23分37秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ioctl.h>
int main(int argc,char *argv[])
{
	int on;
	int led_no;
	int fd;
	//检查led控制的两个参数，如果没有参数输入则退出
	if(argc!=3 || sscanf(argv[1],"%d",&led_no)!=1 || sscanf(argv[2],"%d",&on)!=1 || on<0 || on>1 ||led_no<0 || led_no>4)
  {
    fprintf(stderr, "Usage: leds led_no 0|1\n");
    exit(1);
    fd =open("/dev/led",0);
    if (fd<0)
      perror("open device leds");
      exit(1);
  }
  ioctl(fd,on,led_no);
  close(fd);
  return 0;
}
