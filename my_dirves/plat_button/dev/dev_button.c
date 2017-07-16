/*************************************************************************
    > File Name: dev_button.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年06月29日 星期四 10时30分02秒
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
#include<linux/platform_device.h>

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
static struct resource button_resource[]={
  [0]={
    .start=(resource_size_t)S3C64XX_GPNDAT,
    .end= (resource_size_t)S3C64XX_GPNDAT,
    .flags=IORESOURCE_MEM,
  },
  [1]={
    .start=(resource_size_t)S3C64XX_GPLDAT,
    .end=(resource_size_t)S3C64XX_GPLDAT,
    .flags=IORESOURCE_MEM,
  },
};
struct button_platform
{
  struct button_irq_desc *button_irq;
  int button_num;
};
struct button_platform s3c6410_button={
  .button_irq=button_irqs,
  .button_num=ARRAY_SIZE(button_irqs),
};
struct platform_device s3c6410_button_dev={
  .id=-1,
  .name="s3c6410_button",
  .dev={
    .platform_data=&s3c6410_button,
  },
  .num_resources=ARRAY_SIZE(button_resource),
  .resource=button_resource,
};
static int __init s3c6410_button_dev_init(void)
{
  printk("button_dev init!\n");
  platform_device_register(&s3c6410_button_dev);
  return 0;
}
static void __exit s3c6410_button_dev_exit(void)
{
  printk("button_dev exit!");
  platform_device_unregister(&s3c6410_button_dev);
}
module_init(s3c6410_button_dev_init);
module_exit(s3c6410_button_dev_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("doon");

