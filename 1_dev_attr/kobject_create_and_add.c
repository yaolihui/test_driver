#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define MY_MAJOR 300

dev_t dno = 0;//MKDEV(MY_MAJOR, 0);
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

struct kobject *testinfo_kobj;
int enter(void)
{
	testinfo_kobj = kobject_create_and_add("test_bootinfo", NULL);
	printk("kobject_create_and_add\n ");

	sysfs_create_file(testinfo_kobj, &dev_attr_test.attr);
	printk("sysfs_create_file\n ");
	
  	printk("\n end of %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

void quit(void)
{
	kobject_del(testinfo_kobj);
	printk("%s:kobject_del dno=%d\n ", __func__, dno);
	
	printk("\n end of %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
}

module_init(enter);
module_exit(quit);
MODULE_LICENSE("GPL");

