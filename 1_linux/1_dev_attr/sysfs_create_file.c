#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define MY_MAJOR 300

extern int reg_cdev(int mjr);
extern void unreg_cdev(void);

dev_t dno = 0;//MKDEV(MY_MAJOR, 0);
struct class *cls;
struct device *dev;
char mm[10];

ssize_t test_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	printk("mm=%s\n ", mm);	
	strcpy(buf, mm);
	printk("\ntest_show dev=%p, buf=%s\n ", dev, buf);
	return 10;
}

ssize_t test_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
	strcpy(mm, buf);
	printk("buf=%s\n ", buf);
	mm[1]='$';
	mm[9]='\0';
	printk("\ntest_store dev=%p, mm=%s, count=%d \n", dev, mm, (int)count);
	return 10;
}
DEVICE_ATTR(test, 0660, test_show, test_store);

int enter(void)
{
	printk(KERN_CRIT "\n %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	
	dno = reg_cdev(MY_MAJOR);
	printk(KERN_WARNING "%s:reg_cdev:dno=%d\n ", __func__, dno);


	cls = class_create(THIS_MODULE, "1_class");
	printk("class_creat cls=%p\n ", cls);

	dev = device_create(cls, NULL, dno, NULL, "1_device");
	printk("device_create dev=%p\n ", dev);

	sysfs_create_file(&dev->kobj, &dev_attr_test.attr);
	printk("sysfs_create_file\n ");
	
  	printk("\n end of %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

void quit(void)
{
	device_destroy(cls, dno);
	printk("device_destory, dno=%d\n ", dno);

	class_destroy(cls);
	printk("class_destory, cls=%p\n ", cls);

	unreg_cdev();
	printk("%s:unreg_cdev dno=%d\n ", __func__, dno);
	
	printk("\n end of %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
}

module_init(enter);
module_exit(quit);
MODULE_LICENSE("GPL");

