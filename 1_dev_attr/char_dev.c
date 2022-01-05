#include <linux/fs.h>       // struct file_operations
#include <linux/cdev.h>     // struct cdev


struct file_operations  fops = {
     .owner = THIS_MODULE,
     .open = NULL,
     .read = NULL ,
     .write = NULL,
     .release = NULL,
     .unlocked_ioctl = NULL,
 };
	 
static struct cdev *cdev;
static dev_t devno;
static int major;

int reg_cdev(int mjr)
{
	int ret;

	devno = MKDEV(mjr, 0);
	
	if(major) {
        ret = register_chrdev_region(devno , 1, "1_test_char_dev");
		major = mjr;
   	} else {
        ret = alloc_chrdev_region(&devno , 0, 1, "1_test_char_dev");
        major = MAJOR(devno);
    }
	printk("%s: ret=%d, major=%d, devno=%d\n", __func__, ret, major, devno);
	if(ret < 0) return ret;


	cdev= cdev_alloc();
	cdev->ops = &fops;
	cdev->owner = THIS_MODULE;
	printk("%s: cdev=%p\n", __func__, cdev);
	if(!cdev) {
		ret = -EFAULT;
		unregister_chrdev_region(devno, 1);
	}


	ret = cdev_add(cdev, devno, 1);
	printk("%s: ret=%d, cdev=%p, devno=%d, major=%d\n", __func__, ret, cdev, devno, major);
	
	return devno;
}

void unreg_cdev(void)
{
	cdev_del(cdev);
	printk("%s: cdev_del:cdev=%p\n", __func__, cdev);

	unregister_chrdev_region(devno, 1);
	printk("%s: unregister_chrdev_region:devno=%d, major=%d\n", __func__, devno, major);
	
}

