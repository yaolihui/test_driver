#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static int g_res[10];
static char* g_res_name;
static int g_res_flag;
static int g_res_cnt;

static int probefn(struct platform_device *dev) 
{
	printk("\n enter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	int i = 0;
	struct resource *res;
	while (1) {
		res= platform_get_resource(dev, IORESOURCE_IRQ, i);
		if (!res) {
			break;
		}
		g_res[g_res_cnt] = res->start;
		g_res_flag = res->flags;
		g_res_name = res->name;
		printk("\n platform_get_resource: name=%s, start=%x, flag=0x%x, g_res_cnt=%d \n ", g_res_name, g_res[g_res_cnt], g_res_flag, g_res_cnt);

		int irq = platform_get_irq(dev, i);
		printk("\n platform_get_irq=%x \n ", irq);
	
		i++;
		g_res_cnt++;
	}
	
	char* name = "res_1";
	int irq = platform_get_irq_byname(dev, name);
	printk("\n platform_get_irq_byname irq=%x byname=%s\n ", irq, name);

	res= platform_get_resource_byname( dev, IORESOURCE_MEM, "res_2");
	printk("\n res: platform_get_resource_byname  name=%s, start=%x, flag=0x%x, g_res_cnt=%d \n ", "res_2", g_res[g_res_cnt], g_res_flag, g_res_cnt);
		
	printk("\n exit %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int removefn(struct platform_device *dev)
{
	printk("\n enter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

struct platform_driver pdrv = {
	.probe = probefn,
	.remove = removefn,
	.driver = {
		.name = "test_100_name",
	},
};

static int init_driver_fn(void)
{
	printk("\n enter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	int ret = platform_driver_register(&pdrv);
	printk("\n end %s:%s:%d \n ", __FILE__, __FUNCTION__, __LINE__);
	return ret;
}

static void exit_driver_fn(void)
{
	printk("\n enter %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
	platform_driver_unregister(&pdrv);
	printk("\n end %s:%s:%d\n ", __FILE__, __FUNCTION__, __LINE__);
}

module_init(init_driver_fn);
module_exit(exit_driver_fn);
MODULE_LICENSE("GPL");

