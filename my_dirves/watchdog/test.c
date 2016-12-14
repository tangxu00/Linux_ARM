/*************************************************************************
    > File Name: test.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2016年12月14日 星期三 09时43分44秒
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<stdlib.h>
//#include<linux/watchdog.h>
#define WDIOC_KEEPALIVE 1
#define WDIOC_SETTIMEOUT 2
int main(int argc,const char *argv[])
{
  int fd = open("/dev/wdc", O_WRONLY);
  if (fd==1)
  {
    perror("wdc");
    exit(1);
  }
  int timeout =4;
  ioctl(fd,WDIOC_SETTIMEOUT,&timeout);
  while(1)
  {
    ioctl(fd,WDIOC_KEEPALIVE,0);
    sleep(1);
  }
  close(fd);
}
