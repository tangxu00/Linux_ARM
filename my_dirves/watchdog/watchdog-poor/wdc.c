/*************************************************************************
    > File Name: wdc.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2016年12月13日 星期二 16时28分45秒
 ************************************************************************/

#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>

#include<linux/slab.h>

#include<linux/init.h>
#include<linux/cdev.h>
#include<linux/mm.h>
#include<linux/errno.h>
#include<linux/types.h>
#include<linux/fcntl.h>
#include<linux/version.h>
#include<linux/vmalloc.h>
#include<linux/ctype.h>
#include<linux/pagemap.h>
#include<linux/delay.h>
#include<asm/io.h>
#include<asm/irq.h>
#include<asm/signal.h>
#include<asm/uaccess.h>

#define S3C64XX_PA_WDC (0X7E00400)
#define S3C64XX_WDCON 0
#define S3C64XX_WDDAT 4
#define S3C64XX_WDCNT 8
#define S3C64XX_WDLRINT 12
#define WDIOC_KEEPALIVE  1
#define WDIOC_SETTIMEOUT 2

static void __iomem *s3c_wdc_base;
static int wdc_major = 0;
struct simple_dev 
{
  struct cdev wdc_cdev;
};
struct simple_dev *wdcDev;
static int wdctimeout = 0;

int wdc_open(struct inode *inode,struct file *filp)
{
  //struct simple_dev *dev;
  printk("watchdog open\n");
  //dev = container_of(inode->i_cdev,struct simple_dev,cdev);
  //filp->private_data = dev;
  //int tmp = 0X6731;
  //t_watchdag = (104*64)/66500000 =0.00001s
  wdctimeout = 0xffff;
  writel(wdctimeout,s3c_wdc_base + S3C64XX_WDCNT);
  writel(0X6731,s3c_wdc_base + S3C64XX_WDCON);
  printk("S3C64XX_WDCON 0X%x\n",readl(s3c_wdc_base + S3C64XX_WDCON));
return 0;

}

int wdc_release(struct inode *inode,struct file *filp)
{
  //printk("watchdog realse\n");
  return 0;
}

static int wdc_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_pos)
{
  char wbuf[2];
  //printk("write begin\n");
  if(copy_from_user(wbuf,buf,count))
    return -EFAULT;
      switch(wbuf[0])
      {
  case WDIOC_KEEPALIVE:
      printk("S3C64XX_WDDAT 0X%x\n",readl(s3c_wdc_base + S3C64XX_WDDAT));
      printk("S3C64XX_WDCON 0X%x\n",readl(s3c_wdc_base + S3C64XX_WDCON));
      writel(wdctimeout,s3c_wdc_base + S3C64XX_WDCNT);
      printk("S3C64XX_WDCNT 0X%x\n",readl(s3c_wdc_base + S3C64XX_WDCNT));
      break;
  case WDIOC_SETTIMEOUT:
      wdctimeout = 4*10000;
      printk("wdctimeout %d\n",wdctimeout);
      writel(wdctimeout,s3c_wdc_base + S3C64XX_WDDAT);
      break;
  default:
      break;
      }
      return 0;
}

static long wdc_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
  //void __user *argp = (void __user *)arg;
  //int __user *p = argp;
  if(cmd==WDIOC_KEEPALIVE)
  {
      printk("S3C64XX_WDDAT 0X%x\n",readl(s3c_wdc_base + S3C64XX_WDDAT));
      printk("S3C64XX_WDCON 0X%x\n",readl(s3c_wdc_base + S3C64XX_WDCON));
      writel(wdctimeout,s3c_wdc_base + S3C64XX_WDCNT);
      printk("S3C64XX_WDCNT 0X%x\n",readl(s3c_wdc_base + S3C64XX_WDCNT));
  }
  if(cmd==WDIOC_SETTIMEOUT)
  {
      wdctimeout = arg*10000;
      printk("wdctimeout %d\n",wdctimeout);
      writel(wdctimeout,s3c_wdc_base + S3C64XX_WDDAT);
  }
return -EFAULT;
}
struct file_operations wdc_fops =
{
  .owner = THIS_MODULE,
  .open = wdc_open,
  .write = wdc_write,
  .unlocked_ioctl = wdc_ioctl,
  .release = wdc_release,
};


int __init wdc_init(void)
{
  int ret =0;
  int err =0;
  dev_t devno = MKDEV(wdc_major,0);
  if(wdc_major)
  {
    ret = register_chrdev_region(devno,1,"wdc");

  }
  else 
  {
    ret = alloc_chrdev_region(&devno,0,1,"wdc");
    if(ret<0)
    {
      printk("register_chrdev_region failed!\n");
      goto failure_register_chrdev;
    }
  }
  wdc_major = MAJOR(devno);
  printk("device name is : wdc\n");
  printk("wdc_major = %d \n",wdc_major);
  wdcDev = kmalloc(sizeof(struct simple_dev),GFP_KERNEL);
  if(!wdcDev)
  {
    ret = - ENOMEM;
    goto fail_kmalloc;
  }
  memset(wdcDev,0,sizeof(struct simple_dev));
  cdev_init(&wdcDev->wdc_cdev,&wdc_fops);
  wdcDev->wdc_cdev.owner = THIS_MODULE;
  wdcDev->wdc_cdev.ops = &wdc_fops;
  err = cdev_add(&wdcDev->wdc_cdev,devno,1);
  if(err)
  {
    printk(KERN_NOTICE "error in cdev_add\n");
  }
  s3c_wdc_base = ioremap(S3C64XX_PA_WDC,0XFF);
  if(s3c_wdc_base ==NULL)
  {
    printk( KERN_NOTICE "ioremap ERROR\n");
  }
  return 0;
failure_register_chrdev:
  return ret;
fail_kmalloc:
  return ret;
}

void __exit wdc_exit(void)
{
  cdev_del(&wdcDev->wdc_cdev);
  kfree(wdcDev);
  unregister_chrdev_region(MKDEV(wdc_major,0),1);
  printk("module exit\n");
}
module_init(wdc_init);
module_exit(wdc_exit);
