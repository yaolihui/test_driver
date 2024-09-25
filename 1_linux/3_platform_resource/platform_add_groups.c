#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>

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
DEVICE_ATTR(test1, 0660, test_show, test_store);
DEVICE_ATTR(test2, 0660, test2_show, test2_store);
DEVICE_ATTR(test3, 0660, test_show, test_store);
DEVICE_ATTR(test4, 0660, test2_show, test2_store);

struct attribute *attrs[] = {
	&dev_attr_test1.attr,
	&dev_attr_test2.attr,
	NULL,//!!!!!!!!!!!!!! this is very very very important !!!!!!!!!!!!!!
};

struct attribute_group attr_grp = {
	.attrs = attrs,
};

struct attribute *attrs2[] = {
	&dev_attr_test3.attr,
	&dev_attr_test4.attr,
	NULL,//!!!!!!!!!!!!!! this is very very very important !!!!!!!!!!!!!!
};

struct attribute_group attr_grp2 = {
	.attrs = attrs2,
};
const struct attribute_group *attr_grps[] = {
	&attr_grp,
	&attr_grp2,
	NULL,
};

static struct resource test_resources[] = {
	{
		.name = "res_0",
		.start = 0xFFFFAAAA,
		.flags = IORESOURCE_IRQ,

	},
	{
		.name = "res_1",
		.start = 0xFFFFBBBB,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name = "res_2",
		.start = 0x000000CC,
		.end =   0x00000400,
		.flags = IORESOURCE_MEM,
	},
};

void dev_release(struct device *dev){
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
}

struct platform_device pdev = {
	.name = "test_103_name",
	.num_resources = ARRAY_SIZE(test_resources),
	.resource = test_resources,
	.dev = {						// There MUST be supply, otherwise, get error while rmmod
		.release = dev_release,
	},
};

static int init_device_fn(void)
{
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	device_initialize(&pdev.dev);
	int ret = platform_device_add(&pdev);

	device_add_groups(&pdev.dev, attr_grps);// There inode will create in '/sys/devices/platform/test_103_name'

	printk("\nend %s:%s:%d, num_resources=%d\n ", __FILE__, __FUNCTION__, __LINE__, pdev.num_resources);
	return ret;
}

static void exit_device_fn(void)
{
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	platform_device_unregister(&pdev);
	printk("\nend %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
}

module_init(init_device_fn);
module_exit(exit_device_fn);
MODULE_LICENSE("GPL");
