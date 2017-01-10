/*************************************************************************
    > File Name: i2s.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2016年12月21日 星期三 10时41分59秒
 ************************************************************************/

#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/configfs.h>
#include<linux/module.h>
#include<linux/fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include<linux/miscdevice.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include<mach/gpio-bank-e.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include<mach/map.h>

#define S3C64XX_I2SBASE 0X7F003000
#define S3C64XX_I2SCON  0
#define S3C64XX_I2SMOD  4
#define S3C64XX_I2SFIC  8
#define S3C64XX_I2SPSR  12
#define S3C64XX_I2STXD  16
#define S3C64XX_I2SRXD  20

static void __iomem *s3c_i2s_base;
static void __iomem *clksrc;
static void __iomem *gate_base;
int dev_major =235;
void s3c64xx_i2s_init(void)
{
  u32 gate;
  clksrc=ioremap(0x7e00f01c,0x001);

  __raw_writel(0X33333,S3C64XX_GPECON);//配置gpe为i2s模式
  gate_base=ioremap(0x7e00f030,0x100);
  gate = readl(gate_base + 4)|(3<<15);
  printk("0x%x\n",gate);
  writel(gate,(gate_base+4));
  writel((readl(clksrc)|(3<<10)),clksrc);
  printk("clksrc 0x%x\n",readl(clksrc));
  //u32 i2scon=readl(s3c_i2s_base + S3C64XX_I2SCON);
  //u32 i2smod=readl(s3c_i2s_base + S3C64XX_I2SMOD);
  //u32 i2sfic=readl(s3c_i2s_base + S3C64XX_I2SFIC);
  writel(0x78,(s3c_i2s_base+S3C64XX_I2SCON));
  writel(0x110,(s3c_i2s_base+S3C64XX_I2SMOD));
  printk("S3C64XX_I2SCON 0x%x\n",readl(s3c_i2s_base + S3C64XX_I2SCON));
  printk("S3C64XX_I2SMOD 0x%x\n",readl(s3c_i2s_base + S3C64XX_I2SMOD));
  printk("S3C64XX_I2SFIC 0x%x\n",readl(s3c_i2s_base + S3C64XX_I2SFIC));
  printk("S3C64XX_I2SPSR 0x%x\n",readl(s3c_i2s_base + S3C64XX_I2SPSR));
  printk("S3C64XX_I2SRXD 0x%x\n",readl(s3c_i2s_base + S3C64XX_I2SRXD));
  printk("S3C64XX_GPECON 0x%x\n",readl(S3C64XX_GPECON));
  printk("hclk_gate 0x%x\n",readl(gate_base));
  printk("pclk_gate 0x%x\n",readl(gate_base+4));
  printk("sclk_gate 0x%x\n",readl(gate_base+8));
}
static int i2s_open(struct inode *inode,struct file *filp)
{
  s3c_i2s_base = ioremap(S3C64XX_I2SBASE,0x001);
  if(s3c_i2s_base==NULL)
    return -ENXIO;
  s3c64xx_i2s_init();
  printk("i2s open!\n");
  return 0;
}

static int i2s_release(struct inode *inode,struct file *filp)
{
  printk("i2s i2s release\n");
  return 0;
}

static struct file_operations i2s_fops={
  .owner = THIS_MODULE,
  .open = i2s_open,
  .release = i2s_release,
};

static int __init i2s_init(void)
{
  int ret;
  ret = register_chrdev(dev_major,"i2s",&i2s_fops);
  if(ret<0)
  {
    printk("i2s cant get major number\n");
    return ret;
  }
  printk("i2s module init\n");
  return 0;
  
}

static void __exit i2s_exit(void)
{
  unregister_chrdev(dev_major,"i2s");
  printk("i2s module exit\n");
}

module_init(i2s_init);
module_exit(i2s_exit);
//MODULE_ARUTHOR("DOOON");
MODULE_LICENSE("GPL");
