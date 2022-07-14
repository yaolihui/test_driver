#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <net/sock.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>


#include "fpsensor_spi_tee.h"

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#ifdef CONFIG_MTK_CLKMGR
#include "mach/mt_clkmgr.h"
#endif
#if !defined(CONFIG_MTK_CLKMGR)
#include <linux/clk.h>
#endif

#if USE_PLATFORM_BUS
#include <linux/platform_device.h>
#endif


#if FPSENSOR_WAKEUP_SOURCE
#include <linux/pm_wakeup.h>
#else
#include <linux/wakelock.h>
#endif

#if FPSENSOR_TBASE_COMPATIBLE
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
#include "mt_spi.h"
#include "mt_spi_hal.h"
#include "mt_gpio.h"
#include "mach/gpio_const.h"
#else
#include "fpsensor_spi_def.h"
#endif
//#include <tee_fp.h>
//#include "../../../tee/tkcore/include/linux/tee_fp.h"
#endif

///shangfei add fp_version for fpsensor 2022/5/30 begin
extern char fp_version[32];
//shangfei add fp_version for fpsensor 2022/5/30 end

///yaolihui add fp_id for fpsensor 2022/7/1 begin
#if defined(CONFIG_OTP_SUPPORT) && defined(GET_INFO)
extern char fp_id[64];
static unsigned char info[64];
#endif
//yaolihui add fp_id for fpsensor 2022/7/1 end

/*************************** global variables************************** */
static fpsensor_data_t *g_fpsensor = NULL;
volatile static int fpsensor_balance_spi_clk = 0;

#if FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_MASTER_CLK
extern void mt_spi_enable_master_clk(struct spi_device *spi);
extern void mt_spi_disable_master_clk(struct spi_device *spi);

#elif  FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_CLK
struct mt_spi_t *mt_spi = NULL;
extern void mt_spi_enable_clk(struct mt_spi_t *ms);
extern void mt_spi_disable_clk(struct mt_spi_t *ms);

#elif  FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_CLKMER
extern void enable_clock(MT_CG_PERI_SPI0, "spi");
extern void disable_clock(MT_CG_PERI_SPI0, "spi");
#endif

fpsensor_data_t *fpsensor_dev_glo = NULL;

#if FPSENSOR_WAKEUP_SOURCE
static inline void wakeup_source_prepare(struct wakeup_source *ws, const char *name)
{
    if (ws) {
        memset(ws, 0, sizeof(*ws));
        ws->name = name;
    }
}

static inline void wakeup_source_drop(struct wakeup_source *ws)
{
    if (!ws)
        return;

    __pm_relax(ws);
}

static inline void wakeup_source_init(struct wakeup_source *ws,
                      const char *name)
{
    wakeup_source_prepare(ws, name);
    wakeup_source_add(ws);
}

static inline void wakeup_source_trash(struct wakeup_source *ws)
{
    wakeup_source_remove(ws);
    wakeup_source_drop(ws);
}
#endif

#ifdef	GET_CHIP_INFO
static chipone_chip_info fpsensor_chip_info = {0,0,"unknow"};
#endif

#if FPSENSOR_TBASE_COMPATIBLE
static int tee_spi_transfer(const char *txbuf, char *rxbuf, int len)
{
    struct spi_transfer t;
    struct spi_message m;
    memset(&t, 0, sizeof(t));
    spi_message_init(&m);
    t.tx_buf = txbuf;
    t.rx_buf = rxbuf;
    t.bits_per_word = 8;
    t.len = len;
    t.speed_hz = 1*1000000;
    spi_message_add_tail(&t, &m);
    return spi_sync(g_fpsensor->spi, &m);
}
#endif

/* -------------------------------------------------------------------- */
/* fingerprint chip hardware configuration                              */
/* -------------------------------------------------------------------- */

static DEFINE_MUTEX(spidev_set_gpio_mutex);
static void spidev_gpio_as_int(fpsensor_data_t *fpsensor)
{
    FUNC_ENTRY();
    mutex_lock(&spidev_set_gpio_mutex);
    pinctrl_select_state(fpsensor->pinctrl, fpsensor->eint_as_int);
    mutex_unlock(&spidev_set_gpio_mutex);
    FUNC_EXIT();
}
static int fpsensor_irq_gpio_cfg(fpsensor_data_t *fpsensor)
{
    struct device_node *node;
    u32 ints[2] = {0, 0};
    int status = 0;
    
    spidev_gpio_as_int(fpsensor);

    node = of_find_compatible_node(NULL, NULL, FPSENSOR_DTS_NODE);
    if (node) {
        of_property_read_u32_array( node, "debounce", ints, ARRAY_SIZE(ints));
        // gpio_request(ints[0], "fpsensor-irq");
        // gpio_set_debounce(ints[0], ints[1]);
        fpsensor_debug(DEBUG_LOG,"[fpsensor]ints[0] = %d,is irq_gpio , ints[1] = %d!!\n", ints[0], ints[1]);
        fpsensor->irq_gpio = ints[0];
        fpsensor->irq = irq_of_parse_and_map(node, 0);  // get irq number
        if (!fpsensor->irq) {
            fpsensor_debug(ERR_LOG,"fpsensor irq_of_parse_and_map fail!!\n");
            status = -EINVAL;
        }
        fpsensor_debug(DEBUG_LOG," [fpsensor]fpsensor->irq= %d,fpsensor>irq_gpio = %d\n", fpsensor->irq,
                       fpsensor->irq_gpio);
    } else {
        fpsensor_debug(ERR_LOG,"fpsensor node null !!\n");
        status = -EINVAL;
    }
    return status ;
}

void fpsensor_gpio_output_dts(int gpio, int level)
{
    mutex_lock(&spidev_set_gpio_mutex);
    fpsensor_debug(DEBUG_LOG, "[fpsensor]fpsensor_gpio_output_dts: gpio= %d, level = %d\n", gpio, level);
    if (gpio == FPSENSOR_RST_PIN) {
        if (level) {
            pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_rst_high);
        } else {
            pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_rst_low);
        }
    } 
#if FPSENSOR_SPI_PIN_SET
    else if (gpio == FPSENSOR_SPI_CS_PIN) {
        pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_cs_set);
    } else if (gpio == FPSENSOR_SPI_MO_PIN) {
        pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_mo_set);
    } else if (gpio == FPSENSOR_SPI_CK_PIN) {
        pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_clk_set);
    } else if (gpio == FPSENSOR_SPI_MI_PIN) {
        pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_mi_set);
    }
#endif
#if FPSENSOR_USE_POWER_GPIO
     else if (gpio == FPSENSOR_POWER_PIN) {
        if (level) {
            pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_power_on);
        } else {
            pinctrl_select_state(g_fpsensor->pinctrl, g_fpsensor->fp_power_off);
        }
    } 
#endif 
    mutex_unlock(&spidev_set_gpio_mutex);
}

int fpsensor_gpio_write(int gpio, int value)
{
    fpsensor_gpio_output_dts(gpio, value);
    return 0;
}
int fpsensor_gpio_read(int gpio)
{
    return gpio_get_value(gpio);
}

//shangfei add for config regulator power supply for fpsensor 2022/5/29 begin
static int fpsensor_power_on(fpsensor_data_t *pdata)
{
	int ret = 0;
    fpsensor_debug(ERR_LOG, "fpsensor_power_on\n");
	regulator_set_voltage(pdata->vdd, 3000000, 3000000);
	ret = regulator_enable(pdata->vdd);
	if(ret)
	{
        fpsensor_debug(ERR_LOG, "enable regulato fail\n");
		return ret;
	}
	msleep(10);
	return ret;
}
static int fpsensor_power_off(fpsensor_data_t *pdata)
{
	int ret = 0;
    fpsensor_debug(ERR_LOG, "fpsensor_power_off\n");
	ret = regulator_disable(fpsensor_dev_glo->vdd);
	if(ret)
	{
        fpsensor_debug(ERR_LOG, "disable regulato fail\n");
		return ret;
	}
	msleep(10);
	return ret;
}
//shangfei add for config regulator power supply for fpsensor 2021/5/29 end


int fpsensor_spidev_dts_init(fpsensor_data_t *fpsensor)
{
    struct device_node *node = NULL;
    struct platform_device *pdev = NULL;
    int ret = 0;
	int v = -1;
    node = of_find_compatible_node(NULL, NULL, FPSENSOR_DTS_NODE);
    if (node) {
        pdev = of_find_device_by_node(node);
        if(pdev) {
            fpsensor->pinctrl = devm_pinctrl_get(&pdev->dev);
            if (IS_ERR(fpsensor->pinctrl)) {
                ret = PTR_ERR(fpsensor->pinctrl);
                fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl.\n");
                return ret;
            }
        } else {
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find device.\n");
            return -ENODEV;
        }
        fpsensor->eint_as_int = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_INT_SET);
        if (IS_ERR(fpsensor->eint_as_int)) {
            ret = PTR_ERR(fpsensor->eint_as_int);
            fpsensor_debug(ERR_LOG, "fpsensor Cannot find fp pinctrl eint_as_int!\n");
            return ret;
        }
        fpsensor->fp_rst_low = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_RESET_LOW);
        if (IS_ERR(fpsensor->fp_rst_low)) {
            ret = PTR_ERR(fpsensor->fp_rst_low);
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl fp_rst_low!\n");
            return ret;
        }
        fpsensor->fp_rst_high = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_RESET_HIGH);
        if (IS_ERR(fpsensor->fp_rst_high)) {
            ret = PTR_ERR(fpsensor->fp_rst_high);
            fpsensor_debug(ERR_LOG, "fpsensor Cannot find fp pinctr fp_rst_high!\n");
            return ret;
        }

#if FPSENSOR_SPI_PIN_SET
        fpsensor->fp_cs_set = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_CS_SET);
        if (IS_ERR(fpsensor->fp_cs_set)) {
            ret = PTR_ERR(fpsensor->fp_cs_set);
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl fp_cs_set!\n");
            return ret;
        }

        fpsensor->fp_mo_set = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_MO_SET);
        if (IS_ERR(fpsensor->fp_mo_set)) {
            ret = PTR_ERR(fpsensor->fp_mo_set);
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl fp_mo_set!\n");
            return ret;
        }

        fpsensor->fp_mi_set = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_MI_SET);
        if (IS_ERR(fpsensor->fp_mi_set)) {
            ret = PTR_ERR(fpsensor->fp_mi_set);
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl fp_mi_set!\n");
            return ret;
        }

        fpsensor->fp_clk_set = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_CLK_SET);
        if (IS_ERR(fpsensor->fp_clk_set)) {
            ret = PTR_ERR(fpsensor->fp_clk_set);
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl fp_clk_set!\n");
            return ret;
        }
        
        fpsensor_gpio_output_dts(FPSENSOR_SPI_MO_PIN, 0);
        fpsensor_gpio_output_dts(FPSENSOR_SPI_MI_PIN, 0);
        fpsensor_gpio_output_dts(FPSENSOR_SPI_CK_PIN, 0);
        fpsensor_gpio_output_dts(FPSENSOR_SPI_CS_PIN, 0);
        
#endif

#if FPSENSOR_USE_POWER_GPIO
       fpsensor->fp_power_on = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_POWER_ON);
        if (IS_ERR(fpsensor->fp_power_on)) {
            ret = PTR_ERR(fpsensor->fp_power_on);
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl fp_power_on!\n");
            return ret;
        }

        fpsensor->fp_power_off = pinctrl_lookup_state(fpsensor->pinctrl, FPSENSOR_POWER_OFF);
        if (IS_ERR(fpsensor->fp_power_off)) {
            ret = PTR_ERR(fpsensor->fp_power_off);
            fpsensor_debug(ERR_LOG,"fpsensor Cannot find fp pinctrl fp_clk_set!\n");
            return ret;
        }
        fpsensor_gpio_output_dts(FPSENSOR_POWER_PIN, 1);
            
#endif

    } else {
        fpsensor_debug(ERR_LOG,"fpsensor Cannot find node!\n");
        return -ENODEV;
    }
    return 0;
}
/* delay us after reset */
static void fpsensor_hw_reset(int delay)
{
    FUNC_ENTRY();
    fpsensor_gpio_write(FPSENSOR_RST_PIN,  1);
    udelay(100);
    fpsensor_gpio_write(FPSENSOR_RST_PIN,  0);
    mdelay(1);
    fpsensor_gpio_write(FPSENSOR_RST_PIN,  1);
    if (delay) {
        /* delay is configurable */
        mdelay(delay);
    }
    FUNC_EXIT();
    return;
}



static void fpsensor_spi_clk_enable(u8 bonoff)
{
    if (bonoff == 0 &&(fpsensor_balance_spi_clk == 1)) {
    	#if FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_MASTER_CLK 
    	       mt_spi_disable_master_clk(g_fpsensor->spi);
    	#elif  FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_CLK
    	       mt_spi = spi_master_get_devdata(g_fpsensor->spi->master);
             mt_spi_disable_clk(mt_spi);
    	#elif  FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_CLKMER
    	       disable_clock(MT_CG_PERI_SPI0, "spi");
    	#endif
    	
        fpsensor_balance_spi_clk = 0;
    }
	else if(bonoff == 1&& (fpsensor_balance_spi_clk == 0)) {
		  #if FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_MASTER_CLK
		        mt_spi_enable_master_clk(g_fpsensor->spi);
		  #elif  FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_CLK
		        mt_spi = spi_master_get_devdata(g_fpsensor->spi->master);
    	      mt_spi_enable_clk(mt_spi);
		  #elif  FPSENSOR_SPI_CLOCK_TYPE == FPSENSOR_SPI_CLOCK_TYPE_CLKMER
		        enable_clock(MT_CG_PERI_SPI0, "spi");
		  #endif
   	   
		     fpsensor_balance_spi_clk = 1;
    }
}

static void setRcvIRQ(int val)
{
    fpsensor_data_t *fpsensor_dev = g_fpsensor;
    fpsensor_dev->RcvIRQ = val;
}

static void fpsensor_enable_irq(fpsensor_data_t *fpsensor_dev)
{
    FUNC_ENTRY();
    setRcvIRQ(0);
    /* Request that the interrupt should be wakeable */
    if (fpsensor_dev->irq_enabled == 0) {
        enable_irq(fpsensor_dev->irq);
        fpsensor_dev->irq_enabled = 1;
    }
    FUNC_EXIT();
    return;
}

static void fpsensor_disable_irq(fpsensor_data_t *fpsensor_dev)
{
    FUNC_ENTRY();

    if (0 == fpsensor_dev->device_available) {
        fpsensor_debug(ERR_LOG, "%s, devices not available\n", __func__);
        goto out;
    }

    if (0 == fpsensor_dev->irq_enabled) {
        fpsensor_debug(ERR_LOG, "%s, irq already disabled\n", __func__);
        goto out;
    }

    if (fpsensor_dev->irq) {
        disable_irq_nosync(fpsensor_dev->irq);
        fpsensor_debug(DEBUG_LOG, "%s disable interrupt!\n", __func__);
    }
    fpsensor_dev->irq_enabled = 0;

out:
    setRcvIRQ(0);
    FUNC_EXIT();
    return;
}

static irqreturn_t fpsensor_irq(int irq, void *handle)
{
    fpsensor_data_t *fpsensor_dev = (fpsensor_data_t *)handle;

    /* Make sure 'wakeup_enabled' is updated before using it
    ** since this is interrupt context (other thread...) */
    smp_rmb();
#if FPSENSOR_WAKEUP_SOURCE
	__pm_wakeup_event(&fpsensor_dev->ttw_wl, msecs_to_jiffies(1000));
#else	
    wake_lock_timeout(&fpsensor_dev->ttw_wl, msecs_to_jiffies(1000));
#endif
    setRcvIRQ(1);
    wake_up_interruptible(&fpsensor_dev->wq_irq_return);

    return IRQ_HANDLED;
}

// release and cleanup fpsensor char device
static void fpsensor_dev_cleanup(fpsensor_data_t *fpsensor)
{
    FUNC_ENTRY();

    cdev_del(&fpsensor->cdev);
    unregister_chrdev_region(fpsensor->devno, FPSENSOR_NR_DEVS);
    device_destroy(fpsensor->class, fpsensor->devno);
    class_destroy(fpsensor->class);

    FUNC_EXIT();
}

static long fpsensor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    fpsensor_data_t *fpsensor_dev = NULL;
    int retval = 0;
    unsigned int val = 0;
    int irqf;

    fpsensor_debug(INFO_LOG, "[rickon]: YLH:fpsensor ioctl cmd : 0x%x \n", cmd );
    fpsensor_dev = (fpsensor_data_t *)filp->private_data;
    fpsensor_dev->cancel = 0 ;
    switch (cmd) {
    case FPSENSOR_IOC_INIT:
        fpsensor_debug(INFO_LOG, "%s: fpsensor init started======\n", __func__);
        if(fpsensor_irq_gpio_cfg(fpsensor_dev) != 0) {
        	  retval = -1;
            break;
        }

        irqf = IRQF_TRIGGER_RISING | IRQF_ONESHOT;
        retval = request_threaded_irq(fpsensor_dev->irq, fpsensor_irq, NULL,
                                      irqf, FPSENSOR_DEV_NAME, fpsensor_dev);
        if (retval == 0) {
            fpsensor_debug(INFO_LOG, " irq thread reqquest success!\n");
        } else {
            fpsensor_debug(ERR_LOG, " irq thread request failed , retval =%d \n", retval);
            break;
        }
        enable_irq_wake(g_fpsensor->irq);
        fpsensor_dev->device_available = 1;
        fpsensor_dev->irq_enabled = 1;
        fpsensor_disable_irq(fpsensor_dev);
        fpsensor_debug(INFO_LOG, "%s: fpsensor init finished======\n", __func__);
        break;

    case FPSENSOR_IOC_EXIT:
        fpsensor_disable_irq(fpsensor_dev);
        if (fpsensor_dev->irq) {
            free_irq(fpsensor_dev->irq, fpsensor_dev);
            fpsensor_dev->irq_enabled = 0;
        }
        fpsensor_dev->device_available = 0;
        fpsensor_debug(INFO_LOG, "%s: fpsensor exit finished======\n", __func__);
        break;

    case FPSENSOR_IOC_RESET:
        fpsensor_debug(INFO_LOG, "%s: chip reset command\n", __func__);
        fpsensor_hw_reset(4);
        break;

    case FPSENSOR_IOC_ENABLE_IRQ:
        fpsensor_debug(INFO_LOG, "%s: chip ENable IRQ command\n", __func__);
        fpsensor_enable_irq(fpsensor_dev);
        break;

    case FPSENSOR_IOC_DISABLE_IRQ:
        fpsensor_debug(INFO_LOG, "%s: chip disable IRQ command\n", __func__);
        fpsensor_disable_irq(fpsensor_dev);
        break;
    case FPSENSOR_IOC_GET_INT_VAL:
        val = gpio_get_value(fpsensor_dev->irq_gpio);
        if (copy_to_user((void __user *)arg, (void *)&val, sizeof(unsigned int))) {
            fpsensor_debug(ERR_LOG, "Failed to copy data to user\n");
            retval = -EFAULT;
            break;
        }
        retval = 0;
        break;
    case FPSENSOR_IOC_ENABLE_SPI_CLK:
        fpsensor_debug(INFO_LOG, "%s: ENABLE_SPI_CLK ======\n", __func__);
        fpsensor_spi_clk_enable(1);
        break;
    case FPSENSOR_IOC_DISABLE_SPI_CLK:
        fpsensor_debug(INFO_LOG, "%s: DISABLE_SPI_CLK ======\n", __func__);
        fpsensor_spi_clk_enable(0);
        break;
    case FPSENSOR_IOC_ENABLE_POWER:
        fpsensor_debug(INFO_LOG, "%s: FPSENSOR_IOC_ENABLE_POWER ======\n", __func__);
        break;
    case FPSENSOR_IOC_DISABLE_POWER:
        fpsensor_debug(INFO_LOG, "%s: FPSENSOR_IOC_DISABLE_POWER ======\n", __func__);
        break;
    case FPSENSOR_IOC_REMOVE:
        fpsensor_disable_irq(fpsensor_dev);
        if (fpsensor_dev->irq) {
            free_irq(fpsensor_dev->irq, fpsensor_dev);
            fpsensor_dev->irq_enabled = 0;
        }
        fpsensor_dev->device_available = 0;
        fpsensor_dev_cleanup(fpsensor_dev);
#if FP_NOTIFY
        fb_unregister_client(&fpsensor_dev->notifier);
#endif
        fpsensor_dev->free_flag = 1;
        fpsensor_debug(INFO_LOG, "%s remove finished\n", __func__);
        break;
    case FPSENSOR_IOC_CANCEL_WAIT:
        fpsensor_debug(INFO_LOG, "%s: FPSENSOR CANCEL WAIT\n", __func__);
        wake_up_interruptible(&fpsensor_dev->wq_irq_return);
        fpsensor_dev->cancel = 1;
        break;
#if FP_NOTIFY
    case FPSENSOR_IOC_GET_FP_STATUS :
        val = fpsensor_dev->fb_status;
        fpsensor_debug(INFO_LOG, "%s: FPSENSOR_IOC_GET_FP_STATUS  %d \n",__func__, fpsensor_dev->fb_status);
        if (copy_to_user((void __user *)arg, (void *)&val, sizeof(unsigned int))) {
            fpsensor_debug(ERR_LOG, "Failed to copy data to user\n");
            retval = -EFAULT;
            break;
        }
        retval = 0;
        break;
#endif
    case FPSENSOR_IOC_ENABLE_REPORT_BLANKON:
        if (copy_from_user(&val, (void __user *)arg, 4)) {
            retval = -EFAULT;
            break;
        }
        fpsensor_dev->enable_report_blankon = val;
        break;
    case FPSENSOR_IOC_UPDATE_DRIVER_SN:
        if (copy_from_user(&g_cmd_sn, (unsigned int *)arg, sizeof(unsigned int))) {
            fpsensor_debug(ERR_LOG, "Failed to copy g_cmd_sn from user to kernel\n");
            retval = -EFAULT;
            break;
        }
        break;
#ifdef	GET_CHIP_INFO
	case FPSENSOR_IOC_GET_CHIP_INFO:
        if (copy_from_user(&fpsensor_chip_info,(chipone_chip_info *)arg, sizeof(fpsensor_chip_info))) {
            fpsensor_debug(ERR_LOG, "fpsensor Failed to get fpsensor_chip_info from user\n");
            retval = -EFAULT;
            break;
	        }
        fpsensor_debug(ERR_LOG, "fpsensor get chip info:hardware_id == %x, sensor_id ==%d, version info == %s \n", fpsensor_chip_info.hardware_id, fpsensor_chip_info.sensor_id, fpsensor_chip_info.ver_name);

        ///shangfei add fp_version for fpsensor 2022/5/30 begin
        fpsensor_debug(ERR_LOG, "write chipinfo to fp_version \n");
		memset(fp_version,0,sizeof(fp_version));
        snprintf(fp_version, sizeof(fp_version),"%s","ICNF7318CL-A1");
        ///shangfei add fp_version for fpsensor 2022/5/30 end
        break;
#endif

///yaolihuii add fp_id for fpsensor 2022/7/1 begin
#if defined(CONFIG_OTP_SUPPORT) && defined(GET_INFO)
		case FPSENSOR_IOC_INFO:
		if (copy_from_user(&info,(unsigned char *)arg, sizeof(info))) {
			fpsensor_debug(ERR_LOG,"fpsensor Failed to get FPSENSOR_IOC_INFO from user\n");
			retval = -EFAULT;
			break;
		}
/*
		for(int i =0; i < 64; i++){
			fpsensor_debug(ERR_LOG,"YLH:fpsensor get FPSENSOR_IOC_INFO[%d]= %c\n", i, info[i]);
		}
*/
        fpsensor_debug(ERR_LOG, "YLH:before write module_id to fp_id:[%s]\n", info);
		memset(fp_id, 0, sizeof(fp_id));
        snprintf(fp_id, sizeof(fp_id), "%s", info);
        fpsensor_debug(ERR_LOG, "YLH:after  write module_id to fp_id=%s\n", fp_id);
		break;
#endif
///yaolihuii add fp_id for fpsensor 2022/7/1 end

    default:
        fpsensor_debug(ERR_LOG, "fpsensor doesn't support this command(%d)\n", cmd);
        break;
    }

    //FUNC_EXIT();
    return retval;
}

static long fpsensor_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return fpsensor_ioctl(filp, cmd, (unsigned long)(arg));
}

static unsigned int fpsensor_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int ret = 0;

    ret |= POLLIN;
    poll_wait(filp, &g_fpsensor->wq_irq_return, wait);
    if (g_fpsensor->cancel == 1) {
        fpsensor_debug(ERR_LOG, " cancle\n");
        ret =  POLLERR;
        g_fpsensor->cancel = 0;
        return ret;
    }

    if ( g_fpsensor->RcvIRQ) {
        if (g_fpsensor->RcvIRQ == 2) {
            fpsensor_debug(ERR_LOG, " get fp on notify\n");
            ret |= POLLHUP;
        } else {
            fpsensor_debug(ERR_LOG, " get irq\n");
            ret |= POLLRDNORM;
        }
    } else {
        ret = 0;
    }
    return ret;
}

static int fpsensor_open(struct inode *inode, struct file *filp)
{
    fpsensor_data_t *fpsensor_dev;

    FUNC_ENTRY();
    fpsensor_dev = container_of(inode->i_cdev, fpsensor_data_t, cdev);
    fpsensor_dev->users++;
    fpsensor_dev->device_available = 1;
    filp->private_data = fpsensor_dev;
    FUNC_EXIT();
    return 0;
}

static int fpsensor_release(struct inode *inode, struct file *filp)
{
    fpsensor_data_t *fpsensor_dev;
    int    status = 0;

    FUNC_ENTRY();
    fpsensor_dev = filp->private_data;
    filp->private_data = NULL;

    /*last close??*/
    fpsensor_dev->users--;
    if (fpsensor_dev->users <= 0) {
        fpsensor_debug(INFO_LOG, "%s, disble_irq. irq = %d\n", __func__, fpsensor_dev->irq);
        fpsensor_disable_irq(fpsensor_dev);
    }
    fpsensor_dev->device_available = 0;
    FUNC_EXIT();
    return status;
}

static ssize_t fpsensor_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    fpsensor_debug(ERR_LOG, "Not support read opertion in TEE version\n");
    return -EFAULT;
}

static ssize_t fpsensor_write(struct file *filp, const char __user *buf, size_t count,
                              loff_t *f_pos)
{
    fpsensor_debug(ERR_LOG, "Not support write opertion in TEE version\n");
    return -EFAULT;
}

static const struct file_operations fpsensor_fops = {
    .owner          = THIS_MODULE,
    .write          = fpsensor_write,
    .read           = fpsensor_read,
    .unlocked_ioctl = fpsensor_ioctl,
    .compat_ioctl   = fpsensor_compat_ioctl,
    .open           = fpsensor_open,
    .release        = fpsensor_release,
    .poll           = fpsensor_poll,

};

// create and register a char device for fpsensor
static int fpsensor_dev_setup(fpsensor_data_t *fpsensor)
{
    int ret = 0;
    dev_t dev_no = 0;
    struct device *dev = NULL;
    int fpsensor_dev_major = FPSENSOR_DEV_MAJOR;
    int fpsensor_dev_minor = 0;

    FUNC_ENTRY();

    if (fpsensor_dev_major) {
        dev_no = MKDEV(fpsensor_dev_major, fpsensor_dev_minor);
        ret = register_chrdev_region(dev_no, FPSENSOR_NR_DEVS, FPSENSOR_DEV_NAME);
    } else {
        ret = alloc_chrdev_region(&dev_no, fpsensor_dev_minor, FPSENSOR_NR_DEVS, FPSENSOR_DEV_NAME);
        fpsensor_dev_major = MAJOR(dev_no);
        fpsensor_dev_minor = MINOR(dev_no);
        fpsensor_debug(INFO_LOG, "fpsensor device major is %d, minor is %d\n",
                       fpsensor_dev_major, fpsensor_dev_minor);
    }

    if (ret < 0) {
        fpsensor_debug(ERR_LOG, "can not get device major number %d\n", fpsensor_dev_major);
        goto out;
    }

    cdev_init(&fpsensor->cdev, &fpsensor_fops);
    fpsensor->cdev.owner = THIS_MODULE;
    fpsensor->cdev.ops   = &fpsensor_fops;
    fpsensor->devno      = dev_no;
    ret = cdev_add(&fpsensor->cdev, dev_no, FPSENSOR_NR_DEVS);
    if (ret) {
        fpsensor_debug(ERR_LOG, "add char dev for fpsensor failed\n");
        goto release_region;
    }

    fpsensor->class = class_create(THIS_MODULE, FPSENSOR_CLASS_NAME);
    if (IS_ERR(fpsensor->class)) {
        fpsensor_debug(ERR_LOG, "create fpsensor class failed\n");
        ret = PTR_ERR(fpsensor->class);
        goto release_cdev;
    }

    dev = device_create(fpsensor->class, &fpsensor->spi->dev, dev_no, fpsensor, FPSENSOR_DEV_NAME);
    if (IS_ERR(dev)) {
        fpsensor_debug(ERR_LOG, "create device for fpsensor failed\n");
        ret = PTR_ERR(dev);
        goto release_class;
    }
    FUNC_EXIT();
    return ret;

release_class:
    class_destroy(fpsensor->class);
    fpsensor->class = NULL;
release_cdev:
    cdev_del(&fpsensor->cdev);
release_region:
    unregister_chrdev_region(dev_no, FPSENSOR_NR_DEVS);
out:
    FUNC_EXIT();
    return ret;
}

#if FP_NOTIFY
static int fpsensor_fb_notifier_callback(struct notifier_block* self, unsigned long event, void* data)
{
    int retval = 0;
    static char screen_status[64] = { '\0' };
    struct fb_event* evdata = data;
    unsigned int blank;
    fpsensor_data_t *fpsensor_dev = g_fpsensor;

    fpsensor_debug(INFO_LOG,"%s enter.  event : 0x%x\n", __func__, (unsigned)event);
    if (event != FB_EVENT_BLANK /* FB_EARLY_EVENT_BLANK */) {
        return 0;
    }

    blank = *(int*)evdata->data;
    fpsensor_debug(INFO_LOG,"%s enter, blank=0x%x\n", __func__, blank);

    switch (blank) {
    case FB_BLANK_UNBLANK:
        fpsensor_debug(INFO_LOG,"%s: lcd on notify\n", __func__);
        sprintf(screen_status, "SCREEN_STATUS=%s", "ON");
        fpsensor_dev->fb_status = 1;
        if( fpsensor_dev->enable_report_blankon) {
            fpsensor_dev->RcvIRQ = 2;
            wake_up_interruptible(&fpsensor_dev->wq_irq_return);
        }
        break;

    case FB_BLANK_POWERDOWN:
        fpsensor_debug(INFO_LOG,"%s: lcd off notify\n", __func__);
        sprintf(screen_status, "SCREEN_STATUS=%s", "OFF");
        fpsensor_dev->fb_status = 0;
        break;

    default:
        fpsensor_debug(INFO_LOG,"%s: other notifier, ignore\n", __func__);
        break;
    }

    fpsensor_debug(INFO_LOG,"%s %s leave.\n", screen_status, __func__);
    return retval;
}
#endif

#if FPSENSOR_TBASE_COMPATIBLE
static struct mt_chip_conf fpsensor_spi_conf_mt65xx = {
    .setuptime = 15,
    .holdtime = 15,
    .high_time = 21, 
    .low_time = 21,
    .cs_idletime = 20,
    .ulthgh_thrsh = 0,

    .cpol = 0,
    .cpha = 0,

    .rx_mlsb = 1,
    .tx_mlsb = 1,

    .tx_endian = 0,
    .rx_endian = 0,

    .com_mod = FIFO_TRANSFER,
    .pause = 1,
    .finish_intr = 1,
    .deassert = 0,
    .ulthigh = 0,
    .tckdly = 0,
};

typedef enum {
    SPEED_500KHZ = 500,
    SPEED_1MHZ = 1000,
    SPEED_2MHZ = 2000,
    SPEED_3MHZ = 3000,
    SPEED_4MHZ = 4000,
    SPEED_6MHZ = 6000,
    SPEED_8MHZ = 8000,
    SPEED_KEEP,
    SPEED_UNSUPPORTED
} SPI_SPEED;

void fpsensor_spi_set_mode(struct spi_device *spi, SPI_SPEED speed, int flag)
{
    struct mt_chip_conf *mcc = &fpsensor_spi_conf_mt65xx;

    if (flag == 0) {
        mcc->com_mod = FIFO_TRANSFER;
    } else {
        mcc->com_mod = DMA_TRANSFER;
    }

    switch (speed) {
    case SPEED_500KHZ:
        mcc->high_time = 120;
        mcc->low_time = 120;
        break;
    case SPEED_1MHZ:
        mcc->high_time = 60;
        mcc->low_time = 60;
        break;
    case SPEED_2MHZ:
        mcc->high_time = 30;
        mcc->low_time = 30;
        break;
    case SPEED_3MHZ:
        mcc->high_time = 20;
        mcc->low_time = 20;
        break;
    case SPEED_4MHZ:
        mcc->high_time = 15;
        mcc->low_time = 15;
        break;
    case SPEED_6MHZ:
        mcc->high_time = 10;
        mcc->low_time = 10;
        break;
    case SPEED_8MHZ:
        mcc->high_time = 8;
        mcc->low_time = 8;
        break;
    case SPEED_KEEP:
    case SPEED_UNSUPPORTED:
        break;
    }

    if (spi_setup(spi) < 0) {
        fpsensor_debug(ERR_LOG, "fpsensor:Failed to set spi.\n");
    }
}

/* -------------------------------------------------------------------- */
int fpsensor_spi_setup(fpsensor_data_t *fpsensor)
{
    int ret = 0;

    FUNC_ENTRY();
    fpsensor->spi->mode = SPI_MODE_0;
    fpsensor->spi->bits_per_word = 8;
//    fpsensor->spi->chip_select = 0;
    fpsensor->spi->controller_data = (void *)&fpsensor_spi_conf_mt65xx;
    ret = spi_setup(fpsensor->spi);
    if (ret < 0) {
        fpsensor_debug(ERR_LOG, "spi_setup failed\n");
        return ret;
    }
    fpsensor_spi_set_mode(fpsensor->spi, fpsensor->spi_freq_khz, 0);

    return ret;
}

int fpsensor_detect_hwid(fpsensor_data_t *fpsensor)
{
	unsigned int hwid = 0, status = 0, match= 0, retry = 5;
    unsigned char chipid_tx[4] = {0};
    unsigned char chipid_rx[4] = {0};
	
    fpsensor_debug(ERR_LOG, " ---- fpsensor_detect_hwid.\n");
    fpsensor_hw_reset(4);
	fpsensor->spi_freq_khz = 6000u;
    fpsensor_spi_setup(fpsensor);
    
    do {
	    chipid_tx[0] = 0x08;
	    chipid_tx[1] = 0x55;
        status = tee_spi_transfer(chipid_tx, chipid_rx, 2);
	    if (status != 0) {
	    	fpsensor_debug(ERR_LOG, "%s, tee spi transfer failed, status=0x%x .\n", __func__,status);
		}
		
		chipid_tx[0] = 0x00;
	    chipid_rx[1] = 0x00;
	    chipid_rx[2] = 0x00;
        status = tee_spi_transfer(chipid_tx, chipid_rx, 3);
	    if (status == 0) {
				fpsensor_debug(DEBUG_LOG, "chipid_rx : %x  %x  %x  %x \n",chipid_rx[0],chipid_rx[1],chipid_rx[2],chipid_rx[3]);
				hwid = (chipid_rx[1] << 8) | (chipid_rx[2]);
				if ((hwid == 0x7332) || (hwid == 0x7153) || (hwid == 0x7230) ||(hwid == 0x7222)||(hwid == 0x7312)||(hwid == 0x7339)||(hwid == 0x7318)||(hwid == 0x7319)){
					fpsensor_debug(DEBUG_LOG,"get HWID == 0x%x, is fpsensor.\n",hwid);
					match = 0;
				    break;
				} 
				else{
				   match = 1;
				}
			}
		   else{
			    fpsensor_debug(ERR_LOG, "%s, tee spi transfer failed, status=0x%x ..\n", __func__,status);
				match = 1;
				
		       }	    
		   mdelay(1);	
    }
    while (retry--);
	return match;
}
#endif


#if USE_SPI_BUS
static int fpsensor_probe(struct spi_device *spi)
#elif USE_PLATFORM_BUS
static int fpsensor_probe(struct platform_device *spi)
#endif
{
    int status = 0;
    fpsensor_data_t *fpsensor_dev = NULL;   
    struct device_node *fpsensor_node;
    FUNC_ENTRY();

    //shangfei add for config regulator power supply for fpsensor 2021/5/29 begin
    struct device *dev = &spi->dev;
    fpsensor_node = spi->dev.of_node;
	spi->dev.of_node = of_find_compatible_node(NULL, NULL, FPSENSOR_DTS_NODE);
    if (!spi->dev.of_node) {
        fpsensor_debug(ERR_LOG, "of_find_compatible_node(..) failed.\n");
		goto out;
	}
    struct device_node *np = spi->dev.of_node;

    /* Allocate driver data */
    fpsensor_dev = kzalloc(sizeof(*fpsensor_dev), GFP_KERNEL);
    if (!fpsensor_dev) {
        status = -ENOMEM;
        fpsensor_debug(ERR_LOG, "%s, Failed to alloc memory for fpsensor device.\n", __func__);
        goto out;
    }

    fpsensor_dev->dev = dev;
    dev_set_drvdata(dev, fpsensor_dev);
    //shangfei add for config regulator power supply for fpsensor 2021/5/29 end
    /* Initialize the driver data */
    g_fpsensor = fpsensor_dev;
    fpsensor_dev->spi               = spi ;
    fpsensor_dev->device_available  = 0;
    fpsensor_dev->users             = 0;
    fpsensor_dev->irq               = 0;
    fpsensor_dev->irq_gpio          = 0;
    fpsensor_dev->irq_enabled       = 0;
    fpsensor_dev->free_flag         = 0;

	  status = fpsensor_spidev_dts_init(fpsensor_dev);
	  if (status < 0)
	  {
	  	fpsensor_debug(ERR_LOG, "%s, fpsensor_spidev_dts_init failed.\n", __func__);
	  	goto err1;
	  }

    //shangfei add for config regulator power supply for fpsensor 2022/5/28 begin
    fpsensor_debug(ERR_LOG, "1regulator_get\n");
    g_fpsensor->vdd = regulator_get(&g_fpsensor->spi->dev, "vdd_ana");
    msleep(10);
    fpsensor_power_on(g_fpsensor);
    //shangfei add for config regulator power supply for fpsensor 2022/5/28 end

#if FPSENSOR_TBASE_COMPATIBLE
    fpsensor_spi_clk_enable(1);
    status = fpsensor_detect_hwid(fpsensor_dev);
    fpsensor_spi_clk_enable(0);
    if (0 != status)
    {
    	fpsensor_debug(ERR_LOG, " 1get chip id error.\n");
        spi->dev.of_node = fpsensor_node;
    	goto err2;
    }
#endif
    /* setup a char device for fpsensor */
    status = fpsensor_dev_setup(fpsensor_dev);
    if (status) {
        fpsensor_debug(ERR_LOG, "fpsensor setup char device failed, %d", status);
        goto err2;
    }

    init_waitqueue_head(&fpsensor_dev->wq_irq_return);
#if FPSENSOR_WAKEUP_SOURCE
	wakeup_source_init(&g_fpsensor->ttw_wl , "fpsensor_ttw_wl");
#else	
    wake_lock_init(&g_fpsensor->ttw_wl, WAKE_LOCK_SUSPEND, "fpsensor_ttw_wl");
#endif
    fpsensor_dev->device_available = 1;
#if FP_NOTIFY
    fpsensor_dev->notifier.notifier_call = fpsensor_fb_notifier_callback;
    fb_register_client(&fpsensor_dev->notifier);
#endif

     fpsensor_spi_clk_enable(1);

    fpsensor_debug(INFO_LOG, "%s finished, driver version: %s\n", __func__, FPSENSOR_SPI_VERSION);
    goto out;
    
err2:
	 if(fpsensor_dev->pinctrl != NULL)
	    devm_pinctrl_put(fpsensor_dev->pinctrl);
	  
err1:
	  if(fpsensor_dev!=NULL){
		  kfree(fpsensor_dev);
      fpsensor_dev = NULL;
		}
out:
    FUNC_EXIT();
    return status;
}

#if USE_SPI_BUS
static int fpsensor_remove(struct spi_device *spi)
#elif USE_PLATFORM_BUS
static int fpsensor_remove(struct platform_device *spi)
#endif
{
    fpsensor_data_t *fpsensor_dev = g_fpsensor;

    FUNC_ENTRY();
    fpsensor_disable_irq(fpsensor_dev);
    if (fpsensor_dev->irq)
        free_irq(fpsensor_dev->irq, fpsensor_dev);
#if FP_NOTIFY
    fb_unregister_client(&fpsensor_dev->notifier);
#endif
    fpsensor_dev_cleanup(fpsensor_dev);
#if FPSENSOR_WAKEUP_SOURCE	
	 wakeup_source_trash(&fpsensor_dev->ttw_wl);
#else
    wake_lock_destroy(&fpsensor_dev->ttw_wl);
#endif

     if(fpsensor_dev!=NULL){
		  kfree(fpsensor_dev);
      fpsensor_dev = NULL;
		}

    FUNC_EXIT();
    return 0;
}
#if 0
static int fpsensor_suspend(struct device *dev, pm_message_t state)
{
    return 0;
}

static int fpsensor_resume( struct device *dev)
{
    return 0;
}
#endif
#ifdef CONFIG_OF
static struct of_device_id fpsensor_of_match[] = {
    { .compatible = FPSENSOR_COMPATIBLE_NODE, },
    {}
};
MODULE_DEVICE_TABLE(of, fpsensor_of_match);
#endif

#if USE_SPI_BUS
static struct spi_driver fpsensor_spi_driver = {
    .driver = {
        .name = FPSENSOR_DEV_NAME,
        .bus = &spi_bus_type,
        .owner = THIS_MODULE,
#ifdef CONFIG_OF
        .of_match_table = of_match_ptr(fpsensor_of_match),
#endif
    },
    .probe = fpsensor_probe,
    .remove = fpsensor_remove,
    //.suspend = fpsensor_suspend,
    //.resume = fpsensor_resume,
};
#elif USE_PLATFORM_BUS
static struct platform_driver fpsensor_plat_driver = {
    .driver = {
        .name = FPSENSOR_DEV_NAME,
        .bus    = &platform_bus_type,
        .owner = THIS_MODULE,
#ifdef CONFIG_OF
        .of_match_table = of_match_ptr(fpsensor_of_match),
#endif
    },
    .probe = fpsensor_probe,
    .remove = fpsensor_remove,
    .suspend = fpsensor_suspend,
    .resume = fpsensor_resume,

};
#endif

static int __init fpsensor_init(void)
{
    int status;
#if USE_PLATFORM_BUS
    status = platform_driver_register(&fpsensor_plat_driver);
#elif USE_SPI_BUS
    status = spi_register_driver(&fpsensor_spi_driver);
#endif
    if (status < 0) {
        fpsensor_debug(ERR_LOG, "%s, Failed to register TEE driver.\n", __func__);
    }

    return status;
}
module_init(fpsensor_init);

static void __exit fpsensor_exit(void)
{
#if USE_PLATFORM_BUS
    platform_driver_unregister(&fpsensor_plat_driver);
#elif USE_SPI_BUS
    spi_unregister_driver(&fpsensor_spi_driver);
#endif
}
module_exit(fpsensor_exit);

MODULE_AUTHOR("xhli");
MODULE_DESCRIPTION(" Fingerprint chip TEE driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:fpsensor-drivers");
