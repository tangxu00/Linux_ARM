
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <mach/gpio-bank-b.h>
#include <mach/gpio-bank-e.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <mach/hardware.h>
#include <mach/regs-gpio.h>
#include <mach/map.h>

#define DEBUG_SHOW 0
#define BUFFERSIZE 4
#define DEVICE_NAME  "max"

int dev_MAJOR=235;

#define DEVICE_ADD 0x10
#define READ_DEV_ADDR ((DEVICE_ADD<<1)+1)
#define WRITE_DEV_ADDR (DEVICE_ADD<<1)

#define I2C_READ_DATA  1
#define I2C_WRITE_DATA 2

#define S3C64XX_PA 0X7F003000
#define S3C64XX_IISCON 0
#define S3C64XX_IISMOD 4
#define S3C64XX_IISFIC 8
#define S3C64XX_IISPSR 12
#define S3C64XX_IISTXD 16
#define S3C64XX_IISRXD 20

#define CLK_PA 0X7E00F000
#define EPLL_CON0 0X14
#define EPLL_CON1 0X18
#define CLK_SRC 0X1C
#define CLK_DIV2 0X28
#define PCLK_GATE 0X34
#define SCLK_GATE 0X38
unsigned short int wbuf[640000];

static void __iomem *s3c_iis_base;
static void __iomem *clk_pa;
void s3c64xx_iis_init(void)
{
  u32 gate;
  writel(0x33333,S3C64XX_GPECON);//配置GPE为iis模式

  writel(0x802d0103,(clk_pa+EPLL_CON0)); //配置epll
  writel(0x289e,(clk_pa+EPLL_CON1));//为67.739MHZ
  writel((readl(clk_pa+CLK_SRC)|(1<<2)),(clk_pa+CLK_SRC));//为iis选择EPLL
  writel((readl(clk_pa+CLK_SRC)&(~(7<<10))),(clk_pa+CLK_SRC));//选通EPLL
  writel((readl(clk_pa+CLK_DIV2)&(~(15<<12))),(clk_pa+CLK_DIV2));//设置分频系数为1
  writel((readl(clk_pa+SCLK_GATE)|(1<<9)),(clk_pa+SCLK_GATE));//选通sclk_audio1
  printk("epll_con0 0x%x\n",readl(clk_pa+EPLL_CON0));
  printk("epll_con1 0x%x\n",readl(clk_pa+EPLL_CON1));
  printk("clk_src 0x%x\n",readl(clk_pa+CLK_SRC));
  printk("clk_div2 0x%x\n",readl(clk_pa+CLK_DIV2));
  printk("sclk_gate 0x%x\n",readl(clk_pa+SCLK_GATE));

  gate=readl(clk_pa+PCLK_GATE)|(3<<15);
  writel(gate,clk_pa+PCLK_GATE);//为iis1选通pclk
  //writel((readl(clk_src)|(3<<10)),clk_src);//codeclk
  writel(0x61,(s3c_iis_base + S3C64XX_IISCON));
  writel(0x530,(s3c_iis_base + S3C64XX_IISMOD));
  writel(0,(s3c_iis_base + S3C64XX_IISFIC));
  writel(0x8300,(s3c_iis_base + S3C64XX_IISPSR));
  printk("S3C64XX_IISCON 0x%x\n",readl(s3c_iis_base + S3C64XX_IISCON));
  printk("S3C64XX_IISMOD 0x%x\n",readl(s3c_iis_base + S3C64XX_IISMOD));
  printk("S3C64XX_IISFIC 0x%x\n",readl(s3c_iis_base + S3C64XX_IISFIC));
  printk("S3C64XX_IISPSR 0x%x\n",readl(s3c_iis_base + S3C64XX_IISPSR));
}

struct I2C_MSGbuffer
{
	unsigned short len;
	unsigned char addr;
	char buffer[64];
};

void SetSDAOut(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBCON)&(~(7<<25)))|(1<<24),S3C64XX_GPBCON);
	//__raw_writel((readl(S3C2410_GPEUP)|(1<<15)),S3C2410_GPEUP);
}

void SetSDAInput(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBCON)&(~(15<<24))),S3C64XX_GPBCON);
}

void SetSCLOut(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBCON)&(~(7<<21)))|(1<<20),S3C64XX_GPBCON);
}

void SetSCLInput(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBCON)&(~(15<<20))),S3C64XX_GPBCON);
}

int GetSDAValue(void)
{
	int ret=__raw_readl(S3C64XX_GPBDAT)&(1<<6);
	if(ret==0)
		return 0;
	else
	    return 1;
}

void SetSDAHigh(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBDAT)|(1<<6)),S3C64XX_GPBDAT);
}

void SetSDALow(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBDAT)&(~(1<<6))),S3C64XX_GPBDAT);
}

void SetSCLHigh(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBDAT)|(1<<5)),S3C64XX_GPBDAT);
}

void SetSCLLow(void)
{
	__raw_writel((__raw_readl(S3C64XX_GPBDAT)&(~(1<<5))),S3C64XX_GPBDAT);
}

void I2CStart(void)
{
	SetSDAHigh();
	SetSCLHigh();
    udelay(40);
	SetSDALow();
	udelay(100);
	SetSCLLow();
	udelay(80);
}


void I2CStop(void)
{
	SetSDALow();
	udelay(60);
	SetSCLHigh();
	udelay(80);
	SetSDAHigh();
}

int WaitAck(void)
{
	int isum=2000;
	int ret=0;
	udelay(1);
    SetSDAInput();
	SetSCLHigh();
	udelay(10);
	while(isum>0)
	{
		ret=GetSDAValue();
		if(ret==0)
		{
			break;
		}
		udelay(2);
		isum--;
	}
	SetSCLLow();
	SetSDAOut();
	udelay(2);
	return ret;
}

void SendAck(void)
{
	SetSDALow();
	SetSCLHigh();
	udelay(8);
	SetSCLLow();
	SetSDAHigh();
}


 void SendNotAck(void)
 {
	SetSDAHigh();
	SetSCLHigh();
	udelay(8);
	SetSCLLow();
 }


void I2CSendByte(unsigned char ch)
{
    unsigned char i=8;
      while (i--)
      {
		    if(ch&0x80)
		    {
				SetSDAHigh();
		    }
			else
			{
				SetSDALow();
			}
			udelay(10);
			SetSCLHigh();
			udelay(100);
			SetSCLLow();
			udelay(80);
			ch=ch<<1;
      }
	  SetSDAHigh();
}

unsigned char I2CReceiveByte(void)
{
	  int ret=0;
      unsigned char i=8;
	  unsigned char data=0;
      SetSDAHigh();
      SetSDAInput();
	  udelay(20);
      while (i--)
      {
    	    data<<=1;
			SetSCLHigh();
			udelay(60);
			ret=GetSDAValue();
			data|=ret;
			udelay(20);
			SetSCLLow();
			udelay(60);
		}
	  SetSDAOut();
      return data;
}

int writeOnedate(unsigned char addr,unsigned char ch)
{
	int ret=0;
  //printk("write begin\n");
	I2CStart();
	I2CSendByte(WRITE_DEV_ADDR);
	ret=WaitAck();
#if DEBUG_SHOW
	printk("writedate dev addr ACK:%d\n",ret);
#endif
	if(ret==0)
	{
		I2CSendByte(addr);
		ret=WaitAck();
#if DEBUG_SHOW
		printk("writedate data addr 0x%.2x ACK:%d\n",addr,ret);
#endif
		if(ret==0)
		{
			I2CSendByte(ch);
			ret=WaitAck();
#if DEBUG_SHOW
			printk("writedate data 0x%.2x ACK:%d\n",ch,ret);
#endif
		}
	}
	I2CStop();
	mdelay(10);
	return ret;
}

int readOnedate(unsigned char addr,unsigned char*ch)
{
	int ret=0;
  //printk("read begin\n");
	I2CStart();
	I2CSendByte(WRITE_DEV_ADDR);
	ret=WaitAck();
#if DEBUG_SHOW
	printk("readdate dev addr ACK:%d\n",ret);
#endif
	if(ret==0)
	{
		I2CSendByte(addr);
		ret=WaitAck();
#if DEBUG_SHOW
		printk("readdate data addr 0x%.2x ACK:%d\n",addr,ret);
#endif
		if(ret==0)
		{
			//I2CStop();
			I2CStart();
			I2CSendByte(READ_DEV_ADDR);
			ret=WaitAck();
#if DEBUG_SHOW
			printk("readdate dev addr ACK:%d\n",ret);
#endif
			if(ret==0)
			{
				*ch=I2CReceiveByte();
#if DEBUG_SHOW
				printk("readdate data:0x%.2x\n",*ch);
#endif
				SendNotAck();
			}
		}
	}
	I2CStop();
	return ret;
}

int readThreeTimes(unsigned char addr,unsigned char*ch)
{
	int i=3;
	while(i>0)
	{
		if(readOnedate(addr,ch)==0)
		{
			break;
		}
		i--;
	}
	if(i==0)return 0;
	return 1;
}

int writeThreeTimes(unsigned char addr,unsigned char ch)
{
	int i=3;
	while(i>0)
	{
		if(writeOnedate(addr,ch)==0)
		{
			break;
		}
		i--;
	}
	if(i==0)return 0;
	return 1;
}


static int I2C_open(struct inode *inode, struct file *filp)
{
	int ret=0;

	SetSDAOut();
	SetSCLOut();
  s3c64xx_iis_init();
  printk("#############open##############\n");

	return ret;
}

static int max_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos)
{
  //char wbuf[64000];
  unsigned int i=0;
  //printk("write begin\n");
 // if(copy_from_user(wbuf,buf,count))
   //   return -EFAULT;
  //  printk("FIFO wide %d\n",((readl(s3c_iis_base+S3C64XX_IISFIC))&0x1F));
    //ret=readl(s3c_iis_base+S3C64XX_IISRXD)&0x0000ffff;
    //int wb=0x1111;
    while(i<640000)
    {
      if((readl(s3c_iis_base + S3C64XX_IISCON)&0x200)==0)//FIFO not empty
      {
        wbuf[i]=readl(s3c_iis_base+S3C64XX_IISRXD)&0xffff;
        //wbuf[i+1]=((readl(s3c_iis_base+S3C64XX_IISRXD)&0xff00)>>8);
        i++;
        //wbuf[i]=wb&0xff;
        //wbuf[i+1]=((wb&0xff00)>>8);
        //i=i+2;
        //wb++;
      }
    }
    if(copy_to_user(buf,wbuf,count))
      return -EFAULT;
    //if((readl(s3c_iis_base + S3C64XX_IISCON)&0x200)==0)
    //{
    //  ret=readl(s3c_iis_base+S3C64XX_IISRXD)&0x0000ffff;
    //  printk("%x ",ret);
      //return ret;
    //}
  return 0;
}
static int I2C_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
	case I2C_READ_DATA:
		{
			int i=0;
			struct I2C_MSGbuffer imsg;
      struct I2C_MSGbuffer wmsg;
      memset(&wmsg,0,sizeof(struct I2C_MSGbuffer));
      wmsg.addr=0x03;
      wmsg.len=10;
      //wmsg.buffer[64]={
        //0x11,0xc0,0,0,0x08,0,0x06,0x03,0,0x20,
      //};
      wmsg.buffer[0]=0x11;
      wmsg.buffer[1]=0xc0;
      wmsg.buffer[2]=0;
      wmsg.buffer[3]=0;
      wmsg.buffer[4]=0x09;
      wmsg.buffer[5]=0;
      wmsg.buffer[6]=0x06;
      wmsg.buffer[7]=0x33;
      wmsg.buffer[8]=0;
      wmsg.buffer[9]=0x34;
      for(i=0;i<wmsg.len;i++)
      {
        if(writeThreeTimes(wmsg.addr+i,wmsg.buffer[i])==0)
        {
          break;
        }
      }
      if(writeThreeTimes(0x10,0x83)==0)
      {
        break;
      }
      printk("write %d successfully\n",i);
      printk("ioctl read\n");
			if (copy_from_user(&imsg,(char*)arg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			for(i=0;i<imsg.len;i++)
			{
				if(readThreeTimes(imsg.addr+i,&imsg.buffer[i])==0)
				{
					break;
				}
			}
			if (copy_to_user((char*)arg,&imsg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			return i;
		}
		break;
	case I2C_WRITE_DATA:
		{
			int i=0;
			struct I2C_MSGbuffer imsg;
      printk("ioctl write\n");
			if (copy_from_user(&imsg,(char*)arg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			for(i=0;i<imsg.len;i++)
			{
				if(writeThreeTimes(imsg.addr+i,imsg.buffer[i])==0)
				{
					break;
				}
			}
			return i;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int I2C_release(struct inode *inode,struct file *filp)
{
  printk("###################release###############\n");
	return 0;
}


static struct file_operations I2C_fops={
	.owner = THIS_MODULE,
	.open = I2C_open,
  .read = max_read,
	.unlocked_ioctl = I2C_ioctl,
	.release = I2C_release,
};

static int __init I2C_init(void)
{
	int ret;
	ret = register_chrdev(dev_MAJOR,DEVICE_NAME,&I2C_fops);
	if(ret<0)
	{	
		printk("i2c can't get the major number...\n");
		return ret;
	}

	printk("i2c module init...\n");
  s3c_iis_base = ioremap(S3C64XX_PA,0x100);
  clk_pa = ioremap(CLK_PA,0x100);




	if(s3c_iis_base==NULL)
    	{
      	  printk("error in ioremap\n");
    	}
	return 0;
}
 
static void __exit I2C_exit(void)
{
	unregister_chrdev(dev_MAJOR,DEVICE_NAME);
	printk("i2c module exit...\n");
}	  

module_init(I2C_init);
module_exit(I2C_exit);

MODULE_AUTHOR("dooon <tangxu@gmail.com>");
MODULE_DESCRIPTION("max9860 Test Driver");
MODULE_LICENSE("GPL");
