/*************************************************************************
    > File Name: button.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年06月27日 星期二 14时54分07秒
 ************************************************************************/
#include<linux/fs.h>
#include<linux/module.h>
#include<linux/init.h>
#include<asm/io.h>
#include<linux/cdev.h>
#include<mach/map.h>
#include<mach/regs-gpio.h>
#include<mach/gpio-bank-l.h>
#include<plat/gpio-cfg.h>
#include<mach/gpio-bank-n.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include<asm/uaccess.h>//copy to user
#include<linux/irq.h>
#include<linux/interrupt.h>
#include<linux/device.h>
#include<asm/device.h>
#include<linux/poll.h>

#define DEVICE_NAME "button"
struct button_irq_desc{
  int irq;
  int num;
  char *name;
};
static struct button_irq_desc button_irqs[]={
  {IRQ_EINT(0),0,"KEY0"},
  {IRQ_EINT(1),1,"KEY1"},
  {IRQ_EINT(2),2,"KEY2"},
  {IRQ_EINT(3),3,"KEY3"},
  {IRQ_EINT(4),4,"KEY4"},
  {IRQ_EINT(5),5,"KEY5"},
  {IRQ_EINT(19),6,"KEY6"},
  {IRQ_EINT(20),7,"KEY7"},
};
static volatile int ev_press=0;
static int button_major=0;

struct cdev *button_cdev=NULL;
static volatile unsigned char key_val=0;
static struct class *button_class=NULL;
static struct device *button_device = NULL;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);//声明并初始化一个等待队列

static irqreturn_t button_interrupt(int irq,void *dev_id)
{
  struct button_irq_desc *temp=(struct button_irq_desc*)dev_id;
  udelay(0);
  key_val=(unsigned char)(temp->num+1);
  ev_press =1;
  wake_up_interruptible(&button_waitq);
  return IRQ_RETVAL(IRQ_HANDLED);
}
int button_open(struct inode *inode,struct file *filp)
{
  int i;
  int err;
  for(i=0;i<sizeof(button_irqs)/sizeof(button_irqs[0]);i++)
      {
        if(button_irqs[i].irq<0)
          continue;
        err=request_irq(button_irqs[i].irq,button_interrupt,IRQ_TYPE_EDGE_BOTH,button_irqs[i].name,(void*)&button_irqs[i]);
        if(err)
          break;
      }
  if(err)
  {
    printk("err=%d.\n",err);
    i--;
    for(;i>0;i--)
    {
      if(button_irqs[i].irq<0)
        continue;
      disable_irq(button_irqs[i].irq);
      free_irq(button_irqs[i].irq,(void*)&button_irqs[i]);
    }
    return -EBUSY;
  }
  ev_press=0;
  return 0;
}
int button_close(struct inode *inode,struct file *filp)
{
  int i=0;
  printk("button close!\n");
  for(i=0;i<sizeof(button_irqs)/sizeof(button_irqs[0]);i++)
  {
    disable_irq(button_irqs[i].irq);
    free_irq(button_irqs[i].irq,(void*)&button_irqs[i]);
  }
  return 0;
}
int button_read(struct file *filp,char __user *buf,size_t len,loff_t *pos)
{
  unsigned long val=0;
  if(!ev_press)
  {
    if(filp->f_flags&O_NONBLOCK)//如果指定了非阻塞，则返回-EAGAIN
      return -EAGAIN;
    else
      wait_event_interruptible(button_waitq,ev_press);//若无按键按下，则将当前进程加入到等待队列中并挂起
  }
  ev_press=0;
  if((val=copy_to_user(buf,(const void*)&key_val,sizeof(key_val)))!=sizeof(key_val))
    return -EINVAL;
  return sizeof(key_val);
}
static unsigned int button_poll(struct file *filp,struct poll_table_struct *wait)
{
  unsigned int mask=0;
  if(ev_press)
    mask |=POLLIN | POLLRDNORM;

  return mask;
}
static struct file_operations button_ops={
  .owner =THIS_MODULE,
  .read=button_read,
  .open=button_open,
  .release=button_close,
  .poll=button_poll,
};
static int button_init(void)
{
  int ret;
  int err=0;
  dev_t dev;
  printk("button init!\n");
  if(button_major)
  {
    dev=MKDEV(button_major,0);
    ret=register_chrdev_region(dev,1,DEVICE_NAME);
  }
  else
  {
    ret=alloc_chrdev_region(&dev,0,1,DEVICE_NAME);
    button_major=MAJOR(dev);
  }
  if(ret<0)
    printk(KERN_WARNING"button rregister error!");
  printk("button major is %d\n",button_major);
  button_cdev=cdev_alloc();
  button_cdev->ops=&button_ops;
  cdev_init(button_cdev,&button_ops);
  cdev_add(button_cdev,dev,1);

  button_class=class_create(THIS_MODULE,"tiny6410_button");
  if(IS_ERR(button_class)){
    err=PTR_ERR(button_class);
    printk("create class error.\n");
  }
  button_device=device_create(button_class,NULL,MKDEV(button_major,0),NULL,DEVICE_NAME);
  printk("buttons add ok.\n");
  return 0;
}
static  void button_exit(void)
{
  device_destroy(button_class,MKDEV(button_major,0));
  class_destroy(button_class);
  cdev_del(button_cdev);
  unregister_chrdev_region(MKDEV(button_major,0),1);
  printk("button exit!\n");
}
module_init(button_init);
module_exit(button_exit);
MODULE_AUTHOR("doon");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("button interrupt");
