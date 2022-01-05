#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#define MY_MAJOR 300

dev_t dno;
struct class *cls;
struct device *dev;
char mm[10];

extern int reg_cdev(int);
extern void unreg_cdev(void);

ssize_t test_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	printk("mm=%s\n", mm);	
	strcpy(buf, mm);
	printk("test_show dev=%p, buf=%s\n", dev, buf);
	return 10;
}

ssize_t test_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
	strcpy(mm, buf);
	printk("buf=%s\n", buf);
	mm[5]='$';
	mm[9]='\0';
	printk("test_store dev=%p, mm=%s, count=%d", dev, mm, (int)count);
	return 10;
}
DEVICE_ATTR(test, 0660, test_show, test_store);

int enter(void)
{
    printk("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	dno = reg_cdev(MY_MAJOR);
	printk("reg_cdev dno=%d\n", dno);
	if(dno < 0) return 0;

	cls = class_create(THIS_MODULE,"1_class");
	printk("class_creat cls=%p\n", cls);

	dev = device_create(cls, NULL, dno, NULL, "1_device");
	printk("device_create dev=%p\n", dev);

	sysfs_create_file(&dev->kobj, &dev_attr_test.attr);
	printk("sysfs_create_file\n");

    printk("\nend of enter\n");
    return 0;
}

void quit(void)
{
	device_destroy(cls, dno);
	printk("device_destory\n");

	class_destroy(cls);
	printk("class_destory\n");

	unreg_cdev();
	
    printk("\n===================================\n");
}

module_init(enter);
module_exit(quit);
MODULE_LICENSE("GPL");

