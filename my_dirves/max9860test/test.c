#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define I2C_READ_DATA  1
#define I2C_WRITE_DATA 2

//#define PI M_PI
typedef unsigned int    UI;
typedef unsigned long int   UL;
typedef unsigned short int  US;
typedef unsigned char   UC;
typedef signed int      SI;
typedef signed long int SL;
typedef signed short int    SS;
typedef signed char SC;

#define attr(a) __attribute__((a))
 
#define packed attr(packed)

/* WAV header, 44-byte total */
typedef struct{
 UL riff    packed;
 UL len packed;
 UL wave    packed;
 UL fmt packed;
 UL flen    packed;
 US one packed;
 US chan    packed;
 UL hz  packed;
 UL bpsec   packed;
 US bpsmp   packed;
 US bitpsmp packed;
 UL dat packed;
 UL dlen    packed;
}WAVHDR;

unsigned int fd;
US buf[640000];

struct I2C_MSGbuffer
{
	unsigned short len;
	unsigned char addr;
	char buffer[64];
};


int savefile(const char*const s,const void*const m,const int ml){
 FILE*f=fopen(s,"wb");
 int ok=0;
 if(f){
  ok=fwrite(m,1,ml,f)==ml;
  fclose(f);
 }
 return ok;
}

/* "converts" 4-char string to long int */
#define dw(a) (*(UL*)(a))
 
 
/* Makes 44-byte header for 8-bit WAV in memory
usage: wavhdr(pointer,sampleRate,dataLength) */
 
void wavhdr(void*m,UL hz,UL dlen){
 WAVHDR*p=m;
 p->riff=dw("RIFF");
 p->len=2*dlen+36;
 p->wave=dw("WAVE");
 p->fmt=dw("fmt ");
 p->flen=0x10;
 p->one=1;
 p->chan=1;
 p->hz=hz;
 p->bpsec=2*hz;
 p->bpsmp=2;
 p->bitpsmp=16;
 p->dat=dw("data");
 p->dlen=2*dlen;
}
/*
UC sinewave(UL rate,float freq,UC amp,UL z){
  return sin(z*((PI*2/rate)*freq))*amp+128;
}*/

/* make arbitrary audio data here */
void makeaud(US*p,UL z){
  //float freq=500;
  //UC amp=120;
  UL i=0;
  read(fd,&buf[0],z);
 while(z--){
  *p++=buf[i];
  i++;
 }
}

/* makes wav file */
void makewav(const UL rate,const UL dlen){
 const UL mlen=2*dlen+44;
 US*const m=malloc(mlen);
 if(m){
  wavhdr(m,rate,dlen);
  makeaud(m+22,dlen);
  savefile("out.wav",m,mlen);
 }
}


int main(int argc, char **argv)
{
    //char buf[10]={1,2,3,4,5,6};
    struct I2C_MSGbuffer msg;
   //unsigned int fd;
   unsigned int i,j;
    int ret,wret;
    UL dlen;
    UC*p;
    fd = open("/dev/max", O_RDWR); 
    if (!fd) 
    {     
        printf("opening i2c device Error\n"); 
        return 0; 
    } 
	/*memset(&msg,0,sizeof(struct I2C_MSGbuffer));
	msg.addr=0x00;
	msg.len=5;
	for(i=0;i<5;i++)
	{
		msg.buffer[i]=0x80+i;
	}
	ret=ioctl(fd, I2C_WRITE_DATA, &msg);
	printf("write %d successfully\n",ret); 
*/

	memset(&msg,0,sizeof(struct I2C_MSGbuffer));
	msg.addr=0x03;
	msg.len=14;
	ret=ioctl(fd, I2C_READ_DATA, &msg);
	if(ret>0)
	{
		for(i=0;i<ret;i++)
		{
			printf("add[0x%.2x]=0x%.2x\n",msg.addr+i,msg.buffer[i]); 
		}
	}
	else
	{
		 printf("read Error\n"); 
	}
 /* while(1)
  {
    wret=write(fd,&buf[0],4);
    printf("%x\n",wret);
    sleep(1);
  }*/
  //wret=write(fd,&buf[0],4);
  if(sizeof(WAVHDR)!=44)puts("bad struct");
  makewav(44100,640000);
  //for(j=0;j<1024;j++)
  //  printf("%x ",buf[j]);
  /*dlen=64000000;
  UL mlen=dlen+44;
  UC*m=malloc(mlen);
  if(m){
    wavhdr(m,44100,dlen);
    while(mlen--){
      m=m+44;
      *m++=write(fd,&buf[0],4);
      
    }
  savefile("out.wav",m,mlen);
  }
*/
    close(fd); 
 //   return;
}
