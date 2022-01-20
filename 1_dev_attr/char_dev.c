#include <linux/fs.h>       // struct file_operations
#include <linux/cdev.h>     // struct cdev
#include <linux/uaccess.h>

char mchar[10] = "test";

int test_open(struct inode * i, struct file * f)
{
	printk("%s \n", __func__);
	return 0;
}

ssize_t test_read(struct file * f, char __user * user, size_t count , loff_t *p)
{
	printk("%s, mchar=%s, count=%d, loff_t=%d, strlen=%d \n", __func__, mchar, count, *p, strlen(mchar));
	if (copy_to_user(user, mchar, strlen(mchar))) printk("%s: copy_to_user fail \n", __func__);

	// printk("%s: buf=%s \n", __func__, buf); // buf 为user空间变量，不可读取

	return 0; // 0:表示成功；失败返回没有读取成功的字符数量
}

ssize_t test_write(struct file * f, const char __user * buf, size_t count , loff_t *p)
{
	printk("%s, count=%d, loff_t=%d \n", __func__, count, *p);
 	if (copy_from_user(mchar, buf, count)) printk("%s: copy_from_user fail \n", __func__);
	printk("%s: mchar=%s \n", __func__, mchar);
	return count;
}


struct file_operations  fops = {
     .owner = 	THIS_MODULE,
     .open = 	test_open,
     .read = 	test_read,
     .write = 	test_write,
     .release = NULL,
     .unlocked_ioctl = NULL,
 };
	 
static struct cdev *cdev;
static dev_t devno;
static int major;
static const char* DEV_NAME = "1_test_char_dev";

int reg_cdev(int mjr)
{

	//register_chrdev(mjr, DEV_NAME, &fops);

	devno = MKDEV(mjr, 0xFFFFF/*最大子设备号*/);
	printk("%s: devno=%x, size=%d, major=%d, minor=%d \n", __func__, devno, sizeof(devno), MAJOR(devno), MINOR(devno));

	int ret;
	if(mjr) {
  		ret = register_chrdev_region(devno , 1, DEV_NAME);
		major = mjr;
   	} else {
		ret = alloc_chrdev_region(&devno , 0, 1, DEV_NAME);
		major = MAJOR(devno);
	}
	printk("%s: ret=%d, major=%d, devno=%d \n", __func__, ret, major, devno);
	if(ret < 0) return ret;


	cdev= cdev_alloc();
	cdev->ops = &fops;
	cdev->owner = THIS_MODULE;
	printk("%s: cdev=%p \n", __func__, cdev);
	if(!cdev) {
		ret = -EFAULT;
		unregister_chrdev_region(devno, 1);
	}


	ret = cdev_add(cdev, devno, 1);
	printk("%s: ret=%d, cdev=%p, devno=%d, major=%d \n", __func__, ret, cdev, devno, major);

	return devno;
}

void unreg_cdev(void)
{
	cdev_del(cdev);
	printk("%s: cdev_del:cdev=%p \n", __func__, cdev);

	unregister_chrdev_region(devno, 1);
	printk("%s: unregister_chrdev_region:devno=%d, major=%d \n", __func__, devno, major);
	
}

