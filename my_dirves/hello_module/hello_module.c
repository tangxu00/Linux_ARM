/*************************************************************************
    > File Name: hello_module.c
    > Author: dooon
    > Mail: tangxu314@163.com 
    > Created Time: 2016年11月09日 星期三 16时26分52秒
 ************************************************************************/

#include<linux/init.h>
#include<linux/module.h>
MODULE_LICENSE("dual BSD/GPL");
static int hello_init(void)
{
	printk(KERN_EMERG"HELLO,WORLD!\n");
	return 0;
}
static void hello_exit(void)
{
	printk(KERN_EMERG"GOODBYE WORLD!\N");
}

module_init(hello_init);
module_exit(hello_exit);
