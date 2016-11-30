    #include <linux/module.h>  /*它定义了模块的 API、类型和宏(MODULE_LICENSE、MODULE_AUTHOR等等)，所有的内核模块都必须包含这个头文件。*/   
        
    #include <linux/kernel.h>  /*使用内核信息优先级时要包含这个文件，一般在使用printk函数时使用到优先级信息*/  
      
    #include <linux/fs.h>    
    #include <linux/types.h>
    #include <linux/errno.h>
    #include <linux/init.h>
    #include <linux/cdev.h>
    #include <linux/mm.h>
    #include <asm/uaccess.h> /* copy_to_user,copy_from_user */     
    #include <linux/pci.h>     /*readl writel*/  
    #include <linux/slab.h>
    #include <linux/fcntl.h>
    #include <linux/version.h>
    #include <linux/vmalloc.h>
    #include <linux/ctype.h>
    #include <linux/pagemap.h>
    #include <mach/map.h>     
    #include <mach/regs-gpio.h>      
    #include <mach/gpio-bank-k.h>      
    #include <asm/system.h>
      
        
    //#define LED_MAJOR   243  
      
    #define LED_ON      1  
    #define LED_OFF     0  
    #define LED_1_ON    2  
    #define LED_1_OFF   3  
    #define LED_2_ON    4  
    #define LED_2_OFF   5  
    #define LED_3_ON    6  
    #define LED_3_OFF   7  
    #define LED_4_ON    8  
    #define LED_4_OFF   9  
    
    #define LED_MAJOR 0
    #define LED_NAME "led"
    static int led_major = LED_MAJOR;
    struct simple_dev //led 设备结构体
    {
      struct cdev led_cdev;//cdev结构体
    };
    struct simple_dev *ledDev;

    static int led_open (struct inode *inode,struct file *filp)    
        
    {    
        unsigned tmp;       
      
        tmp = readl(S3C64XX_GPKCON);      
        tmp = (tmp&0x0000ffff)| 0x1111ffff;  
        writel(tmp, S3C64XX_GPKCON);     
      
        printk("#########open######\n");    
        return 0;    
    }    
        
    static int led_read (struct file *filp, char __user *buf, size_t count,loff_t *f_pos)    
    {      
        return count;    
    }    
        
        
    static int led_write (struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)  
      
    {    
        char wbuf[10];    
        unsigned tmp;       
        printk("write begin\n");
          
        if(copy_from_user(wbuf,buf,count))  
            return -EFAULT;    
      
            switch(wbuf[0])    
            {    
              
            case LED_ON:   
                    tmp = readl(S3C64XX_GPKDAT);       
                tmp &= (0x0f);       
                writel(tmp, S3C64XX_GPKDAT);  
                printk("turn on!\n");      
                    break;  
      
            case LED_OFF:    
                tmp  = readl(S3C64XX_GPKDAT);       
                tmp |= (0xf0);       
                writel(tmp, S3C64XX_GPKDAT);   
                printk("turn off!\n");     
                break;             
      
            case LED_1_ON:    
                tmp = readl(S3C64XX_GPKDAT);       
                tmp &= (0xef);       
                writel(tmp, S3C64XX_GPKDAT);   
                printk("turn off!\n");     
                break;    
      
            case LED_1_OFF:   
                    tmp = readl(S3C64XX_GPKDAT);       
                tmp |= (0xf0);       
                writel(tmp, S3C64XX_GPKDAT);  
                printk("turn on!\n");      
                    break;    
      
            case LED_2_ON:    
                tmp = readl(S3C64XX_GPKDAT);       
                tmp &= (0xdf);       
                writel(tmp, S3C64XX_GPKDAT);   
                printk("turn off!\n");     
                break;    
      
            case LED_2_OFF:   
                    tmp = readl(S3C64XX_GPKDAT);       
                tmp |= (0xf0);       
                writel(tmp, S3C64XX_GPKDAT);  
                printk("turn on!\n");      
                    break;    
      
            case LED_3_ON:    
                tmp = readl(S3C64XX_GPKDAT);       
                tmp &= (0xbf);       
                writel(tmp, S3C64XX_GPKDAT);   
                printk("turn off!\n");     
                break;    
      
            case LED_3_OFF:   
                    tmp = readl(S3C64XX_GPKDAT);       
                tmp |= (0xf0);       
                writel(tmp, S3C64XX_GPKDAT);  
                printk("turn on!\n");      
                    break;    
      
            case LED_4_ON:    
                tmp = readl(S3C64XX_GPKDAT);       
                tmp &= (0x7f);       
                writel(tmp, S3C64XX_GPKDAT);   
                printk("turn off!\n");     
                break;    
      
            case LED_4_OFF:   
                    tmp  = readl(S3C64XX_GPKDAT);       
                tmp |= (0xf0);       
                writel(tmp, S3C64XX_GPKDAT);  
                printk("turn on!\n");      
                    break;    
              
      
            default :    
                    break;    
            }    
         return 0;    
    }    
        
    int led_release (struct inode *inode, struct file *filp)    
    {    
        printk("#########release######\n");    
        return 0;    
    }    
        
    struct file_operations led_fops =  
    {    
        .owner = THIS_MODULE,    
        .open = led_open,    
        .read = led_read,    
        .write = led_write,    
        .release = led_release,    
    };    
    /*
    static void led_setup_cdev(struct ledDev *dev,int minor)
    {
      int err;
      devno = MKDEV(led_major,minor);
      cdev_init(&dev->led_cdev,&led_fops);
      dev->led_cdev.owner = THIS_MOUDLE;
      dev->led_cdev>ops = &led_cdev;
      err = cdev_add(&dev->led_cdev,devno,1);//将cdev注册到系统
      if(err)
        printk(KERN_NOTICE "Error in cdev_add()\n");
    }
    */
    int __init led_init (void)    
    {   int ret = 0;
        int err = 0;
        dev_t devno = MKDEV(led_major,0);//构建设备号
        if(led_major)
        {
          ret = register_chrdev_region(devno,1,LED_NAME);//手动分配设备号
        }
        else
        {
          //动态分配设备号
          ret = alloc_chrdev_region(&devno,0,1,LED_NAME);
          if(ret<0)
          {
            printk("register_chrdev_region failed!\n");
            goto failure_register_chrdev;
          }
        }
        //获取设备号
        led_major = MAJOR(devno);
        printk("led namme is: %s!\n",LED_NAME);
        printk("led_major = %d\n",led_major);
        ledDev = kmalloc(sizeof(struct simple_dev),GFP_KERNEL);//动态申请设备结构体内存
        if(!ledDev)//申请失败
        {
          ret = - ENOMEM;
          goto fail_kmalloc;
        }
        memset(ledDev,0,sizeof(struct simple_dev));//将内存清零
        cdev_init(&ledDev->led_cdev,&led_fops);//初始化cdev结构体
        ledDev->led_cdev.owner = THIS_MODULE; //使驱动属于该模块
        ledDev->led_cdev.ops = &led_fops; //cdev连接file_operations指针
        err = cdev_add(&ledDev->led_cdev,devno,1);//将cdev注册到系统
        if(err)
        {
          printk(KERN_NOTICE "Error in cdev_add()\n"); 
        }
        //led_setup_dev(ledDev,0);
        return 0;
failure_register_chrdev:
        return ret;
fail_kmalloc:
        return ret;
    }    
        
    void __exit led_exit (void)    
    {    
        cdev_del(&ledDev->led_cdev);//注销cdev
        kfree(ledDev);//释放结构体内存
        unregister_chrdev_region(MKDEV(led_major,0),1);//释放设备号
        printk ("module exit\n");    
        return ;    
    }    
        
    module_init(led_init);    
    module_exit(led_exit);    
