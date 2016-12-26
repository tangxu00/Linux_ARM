/*************************************************************************
    > File Name: test.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2016年12月21日 星期三 19时49分05秒
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
int main(int argc,const char *argv[])
{
  int fd= open("/dev/i2s",O_WRONLY);
  if(fd==1)
  {
    perror("i2s");
    exit(1);
  }
}
