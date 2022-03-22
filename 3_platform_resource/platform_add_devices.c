#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>


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
	.name = "test_102_name",
	.num_resources = ARRAY_SIZE(test_resources),
	.resource = test_resources,
	.dev = {						// There MUST be supply, otherwise, get error while rmmod
		.release = dev_release,
	},
};

struct platform_device *pdevs[] = {
	&pdev,
};

static int init_device_fn(void)
{
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	int ret = platform_add_devices(pdevs, ARRAY_SIZE(pdevs)); // call to platform_device_register
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
