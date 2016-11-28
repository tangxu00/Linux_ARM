/*************************************************************************
    > File Name: led_driver.c
    > Author: dooon
    > Mail: tangxu314@163.com 
    > Created Time: 2016年11月10日 星期四 15时20分05秒
 ************************************************************************/

#include<linux/module.h>//module_author,module_license
#include<linux/init.h>//module_init,module_exit
#include<linux/fs.h>//file_operations
#include<linux/miscdevice.h>//misdevice
#include<asm/io.h>//ioread32,iowrite32
#include<mach/gpio-bank-k.h>//define GPKCON
#include<mach/regs-gpio.h>//define gpio-bank-k S3C6410_GPK_BASE
#include<mach/map.h>//define S3C64XX_VA_GPIO
#include<linux/kernel.h>

#define DEVICE_NAME "led"

//ioctl 接口函数，cmd=0,表示关闭参数arg指定的led灯，arg的值不能大于4
//其中0表示所有的led
//1～4分别表示led1～led4
//返回0或错误-EINVAL

static int s3c6410_led_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		unsigned tmp;
		case 0://close
		case 1://open
			if(arg>4)
				return -EINVAL;
			else if(arg==0)//全亮或全灭
			{
				tmp=ioread32(S3C64XX_GPKDAT);//读出led1～led4所在寄存器值
				tmp&=~(0xF<<4);//打开led1～led4
			}
			else
			{
				tmp=ioread32(S3C64XX_GPKDAT);
				tmp &=~(1<<(4+arg-1));//清除arg所指示的那一位值
				tmp |=((!cmd)<<(4+arg-1));//写入新值
				iowrite32(tmp,S3C64XX_GPKDAT);
			}
			return 0;
		default:
			return -EINVAL;
	}
}
static struct file_operations dev_fops={
	.owner = THIS_MODULE,
	.unlocked_ioctl = s3c6410_led_ioctl,
};
static struct miscdevice misc ={
	.minor=MISC_DYNAMIC_MINOR,//动态分配次设备号
	.name = DEVICE_NAME,//设备名称
	.fops = &dev_fops,
};
static int __init dev_init(void)
{
	int ret;
	//在设备驱动程序注册时初始化GPIOK为输出模式，并关闭led1～led4
	unsigned tmp;
	tmp = ioread32(S3C64XX_GPKDAT);
	tmp = (tmp & ~(0xFFFFU<<16)) | (0x1111U<<16);//先清除再设为输出
	iowrite32(tmp,S3C64XX_GPKCON);//写入GPKCON
	tmp=ioread32(S3C64XX_GPKDAT);
	tmp |=(0xF<<4);//关闭led灯
	iowrite32(tmp,S3C64XX_GPKDAT);
	//注册misc
	ret = misc_register(&misc);
	printk(DEVICE_NAME"\tinitialized.\n");
	return ret;

}

static void __exit dev_exit(void)
{
	misc_deregister(&misc);
}
module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DOOON");

