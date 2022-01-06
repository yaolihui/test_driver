#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define MY_MAJOR 303

dev_t dno = MKDEV(MY_MAJOR, 0);
struct class *cls;
struct device *dev;
char mm[10];

ssize_t test_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return 10;
}

ssize_t test_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
	return 1;
}

ssize_t test2_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	printk("mm=%s\n", mm);	
	strcpy(buf, mm);
	printk("test_shows dev=%p, buf=%s\n", dev, buf);
	return 10;
}

ssize_t test2_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
	strcpy(mm, buf);
	printk("buf=%s\n", buf);
	mm[5]='$';
	mm[9]='\0';
	printk("test_store2 dev=%p, mm=%s, count=%d", dev, mm, (int)count);
	return 10;

}
DEVICE_ATTR(test, 0660, test_show, test_store);
DEVICE_ATTR(test2, 0660, test2_show, test2_store);

struct attribute *attrs[] = {
	&dev_attr_test.attr,
	&dev_attr_test2.attr,
	NULL,//!!!!!!!!!!!!!! this is very very very important !!!!!!!!!!!!!!
};

struct attribute_group attr_grp = {
	.attrs = attrs,
};

int enter(void)
{
	printk("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	cls = class_create(THIS_MODULE,"3_class");
	printk("class_creat cls=%p\n", cls);

	dev = device_create(cls, NULL, dno, NULL, "3_device");
	printk("device_create dev=%p\n", dev);

	device_add_group(dev, &attr_grp);
	printk("device_add_group\n");

    	printk("end of enter\n");
	return 0;
}

void quit(void)
{
	device_destroy(cls, dno);
	printk("device_destory\n");

	class_destroy(cls);
	printk("class_destory\n");
    
    printk("\n===================================\n");
}

module_init(enter);
module_exit(quit);
MODULE_LICENSE("GPL");

