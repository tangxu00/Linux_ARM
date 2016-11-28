    #include <linux/module.h>  /*它定义了模块的 API、类型和宏(MODULE_LICENSE、MODULE_AUTHOR等等)，所有的内核模块都必须包含这个头文件。*/   
        
    #include <linux/kernel.h>  /*使用内核信息优先级时要包含这个文件，一般在使用printk函数时使用到优先级信息*/  
      
    #include <linux/fs.h>    
    #include <asm/uaccess.h> /* copy_to_user,copy_from_user */     
    #include <linux/pci.h>     /*readl writel*/  
    #include <mach/map.h>     
    #include <mach/regs-gpio.h>      
    #include <mach/gpio-bank-k.h>      
      
        
    #define LED_MAJOR   243  
      
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
        
    int __init led_init (void)    
    {     
        int rc;    
        printk ("Test led dev\n");    
        rc = register_chrdev(LED_MAJOR,"led",&led_fops);   
       
        if (rc <0)    
        {    
            printk ("register %s char dev error\n","led");    
            return -1;    
        }    
        printk ("ok!\n");    
        return 0;    
    }    
        
    void __exit led_exit (void)    
    {    
        unregister_chrdev(LED_MAJOR,"led");    
        printk ("module exit\n");    
        return ;    
    }    
        
    module_init(led_init);    
    module_exit(led_exit);    
