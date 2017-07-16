/*************************************************************************
    > File Name: test.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年06月28日 星期三 15时32分49秒
 ************************************************************************/

#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/select.h>
#include<sys/time.h>
#include<errno.h>
#include<stdio.h>
static int cnt=0;
int main()
{
  int fd;
  unsigned char button=0;
  fd=open("/dev/button",0);
  if(fd<0){
    perror("open device button\n");
    return -1;
  }
  while(1)
  {
    read(fd,&button,sizeof(button));
    printf("%dKEY%02x entered.\n",cnt++,button);
  }
  close(fd);
  return 0;
}

