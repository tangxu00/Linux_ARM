
#ifndef _simple_H_
#define _simple_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/********************************************************
 * Macros to help debugging
 ********************************************************/
#undef PDEBUG             /* undef it, just in case */
#ifdef simple_DEBUG
#ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "DEMO: " fmt, ## args)
#else//usr space
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

//�豸��
#define simple_MAJOR 224
#define simple_MINOR 0

#define WDIOC_KEEPALIVE  1
#define WDIOC_SETTIMEOUT 2

//�豸�ṹ
struct simple_dev 
{
	struct cdev cdev;	  /* Char device structure		*/
};

//��������
ssize_t simple_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);
ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);
loff_t  simple_llseek(struct file *filp, loff_t off, int whence);
//int     simple_ioctl(struct inode *inode, struct file *filp,
//                    unsigned int cmd, unsigned long arg);


#endif /* _simple_H_ */
