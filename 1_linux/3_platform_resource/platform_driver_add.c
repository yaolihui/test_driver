#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static int g_res[10];
static const char* g_res_name;
static int g_res_flag;
static int g_res_cnt;

static int probefn(struct platform_device *dev) 
{
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);

	struct resource *res = dev->resource;
	printk("resource=%p, name=%s, num=%d, %s:%s:%d\n ", dev->resource, dev->name, dev->num_resources, __FILE__, __FUNCTION__, __LINE__);
	for(int i = 0; i < dev->num_resources; i++)
	{
		res= platform_get_resource(dev, IORESOURCE_IRQ, i);
		if (!res) {
			break;
		}
		g_res[g_res_cnt] = res->start;
		g_res_flag = res->flags;
		g_res_name = res->name;
		printk("platform_get_resource: name=%s, start=%x, flag=0x%x, g_res_cnt=%d \n ", g_res_name, g_res[g_res_cnt], g_res_flag, g_res_cnt);

		int irq = platform_get_irq(dev, i);
		printk("platform_get_irq=%x, i=%d \n ", irq, i);
	
		g_res_cnt++;
	}
	
	char* name = "res_1";
	int irq = platform_get_irq_byname(dev, name);
	printk("\nplatform_get_irq_byname irq=%x byname=%s\n ", irq, name);

	res= platform_get_resource_byname( dev, IORESOURCE_MEM, "res_2");
	printk("platform_get_resource_byname  name=%s, start=%x, flag=0x%x, g_res_cnt=%d \n ", "res_2", g_res[g_res_cnt], g_res_flag, g_res_cnt);
		
	printk("\nexit %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int removefn(struct platform_device *dev)
{
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

struct platform_driver pdrv = {
	.probe = probefn,
	.remove = removefn,
	.driver = {
		.name = "test_101_name",
	},
};

static int init_driver_fn(void)
{
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	int ret = platform_driver_register(&pdrv);
	printk("\nend %s:%s:%d \n ", __FILE__, __FUNCTION__, __LINE__);
	return ret;
}

static void exit_driver_fn(void)
{
	printk("\nenter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	platform_driver_unregister(&pdrv);
	printk("\nend %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
}

module_init(init_driver_fn);
module_exit(exit_driver_fn);
MODULE_LICENSE("GPL");

