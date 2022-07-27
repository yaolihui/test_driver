
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <video/mmp_disp.h>
//#include "lcm/inc/lcm_drv.h"
//#include "../misc/mediatek/video/mt6765/videox/disp_recovery.h"
//#include "../misc/mediatek/video/mt6765/dispsys/ddp_info.h"
//#include "kd_imgsensor.h"

//+add by hzb
//#include <../misc/mediatek/include/mt-plat/mt_gpio.h>
//#include "../misc/mediatek/include/mt-plat/mt6755/include/mach/gpio_const.h" 
//#include "upmu_common.h"
//#include <mach/upmu_sw.h>
#include <linux/delay.h>
//-add by hzb

//#include "sec_boot_lib.h"

//extern LCM_DRIVER *lcm_kernel_detect_drv;//add by liuwei
//extern ssize_t modem_show(struct kobject *kobj, struct kobj_attribute *attr, char* buf);

//extern int ontim_torch_onoff(int brightness_level);

    
static struct kobject *bootinfo_kobj = NULL;

const u8 * sub_front_camera[]={"sub_front_camera not found!","RMN1_S5K4H7_SEASONS"};
const u8 * sub_front2_camera[]={"sub_front2_camera not found!"};
const u8 * main_camera[]={"main_back_camera not found!","RMN1_S5KJN1_TS","RMN2_S5KJN1_SUNWIN"};
const u8 * main2_camera[]={"back2_camera not found!","RMN1_GC02M0B_CXT","RMN2_GC02M0B_UNION"};
const u8 * main3_camera[]={"back3_camera not found!","RMN1_GC02M1_SEASONS"};
const u8 * camera_status[]={"ok!", "fail!"};

int back_camera_find_success=0;
int front_camera_find_success=0;
int back2_camera_find_success=0;
int front2_camera_find_success=0;
int back3_camera_find_success=0;
int torch_flash_level=0;
//songzhen DATA20200609 Romanee modify for camera checksum
bool back_camera_otp_status = 1;
bool front_camera_otp_status = 1;
bool back2_camera_otp_status = 1;
bool front2_camera_otp_status = 0;
bool back3_camera_otp_status = 1;
//songzhen DATA20200609 Romanee modify for camera checksum end

//songzhen DATA20200629 Romanee modify for camera msn begin
char back_camera_msn[64] = {0};
char front_camera_msn[64] = {0};
char macro_camera_msn[64] = {0};
//songzhen DATA20200629 Romanee modify for camera msn end

u32 cam_err_code = 0x00000000;



//int lcd_find_success=0;
#if 1
bool tp_probe_ok;//bit0
bool camera_front_probe_ok;//bit1
bool camera_back_probe_ok;//bit2
bool gsensor_probe_ok;//bit3
bool proximity_probe_ok;//bit4
bool charger_probe_ok;//bit5
bool pmu_probe_ok=1;//bit6
bool compass_probe_ok;//bit7
bool camera_back2_probe_ok;//bit9
bool camera_back3_probe_ok;//bit10
bool camera_front2_probe_ok;//bit11
bool fingerprint_probe_ok;//bit31
bool sar_probe_ok;//bit32
#endif

// add by fanjie for tp, start
char touch_version[32] = "tp not found";
char touch_info[32] = "tp not found";
EXPORT_SYMBOL(touch_version);
EXPORT_SYMBOL(touch_info);

static ssize_t touch_version_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",  touch_version);
	return (s - buf);
}

static ssize_t touch_version_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute touch_version_attr = {
	.attr = {
		.name = "tp_version",
		.mode = 0644,
	},
	.show =&touch_version_show,
	.store= &touch_version_store,
};

static ssize_t touch_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",  touch_info);
	return (s - buf);
}

static ssize_t touch_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute touch_info_attr = {
	.attr = {
		.name = "tp_info",
		.mode = 0644,
	},
	.show =&touch_info_show,
	.store= &touch_info_store,
};
// add by fanjie for tp, end

//shangfei add for fp version 2022-5-30 begin
char fp_version[32] = "fp not found";
EXPORT_SYMBOL(fp_version);
static ssize_t fp_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",fp_version);
 	return (s - buf);
 }

static ssize_t fp_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute fp_info_attr = {
	.attr = {
		.name = "fp_info",
		.mode = 0644,
	},
	.show =&fp_info_show,
	.store= &fp_info_store,
};
//shangfei add for fp version 2022-5-30 end

//yaolihui add for module id 20220624
char fp_id[64] = "fp not found";
EXPORT_SYMBOL(fp_id);
static ssize_t fp_id_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",fp_id);
	printk(KERN_ERR "YLH:fp_id_show buf=[%s]", fp_id);
 	return (s - buf);
}

static ssize_t fp_id_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	printk(KERN_ERR "YLH:fp_id_store buf=[%s], n=%d", buf, n);
	int len = sizeof(fp_id);
	int last = n < len - 1 ? n : len - 1;
	strcpy(fp_id, buf);
	fp_id[last] = '\0';
	printk(KERN_ERR "YLH:fp_id_store fp_id=[%s], last=%d", fp_id, last);
	return last;
}
static struct kobj_attribute fp_id_attr = {
	.attr = {
		.name = "fp_id",
		.mode = 0644,
	},
	.show =&fp_id_show,
	.store= &fp_id_store,
};
//yaolihui add for module id  20220624 end

char romanee_lcd_type[32] = "none";
EXPORT_SYMBOL(romanee_lcd_type);
static ssize_t lcd_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",romanee_lcd_type);
	return (s - buf);
}

static ssize_t lcd_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute lcd_info_attr = {
	.attr = {
		.name = "lcd_info",
		.mode = 0644,
	},
	.show =&lcd_info_show,
	.store= &lcd_info_store,
};

extern char *saved_command_line;
#define QR_CODE_NUM 21
static char qr_info[QR_CODE_NUM+1] = {0};

static ssize_t read_lcm_qr_info(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	int i = 0;
	char* tmp = NULL;
	char *s = buf;
	tmp = strstr(saved_command_line, "lcd.qr_info=");
	if (tmp == NULL) {
		s += sprintf(s, "lcm not detect\n");
		return (s - buf);
	}

	tmp += strlen("lcd.qr_info=");
	for (i = 0; i < QR_CODE_NUM; i++)
	{
		qr_info[i] = tmp[i];
	}
	printk(KERN_ERR "qr info is [%s]", qr_info);
	s += sprintf(s, "%s\n",qr_info);
	return (s - buf);
}

static struct kobj_attribute lcm_qr_info_attr = {
	.attr = {
		.name = "lcm_qr_info",
		.mode = 0444,
	},
	.show =&read_lcm_qr_info,
};

/*
extern struct ddp_lcm_read_cmd_table read_table;
extern struct ddp_lcm_write_cmd_table write_table;
extern int do_lcm_vdo_lp_read(struct ddp_lcm_read_cmd_table *read_table);
extern int do_lcm_vdo_lp_write(struct ddp_lcm_write_cmd_table *write_table,
			unsigned int count);
extern void primary_display_esd_check_enable(int enable);
*/
//ssize_t lcd_reg_read_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
//{
////ssize_t ret = 0;
//return sprintf(buf,"value = 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x\n",read_table.data[0],read_table.data[1],read_table.data[2],read_table.data[3],read_table.data[4],read_table.data[5],read_table.data[6],read_table.data[7],read_table.data[8],read_table.data[9]);
//}
//
//ssize_t lcd_reg_read_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
//{
//ssize_t ret=0;
//char rw;
//int i = 0;
//char* cur;
//char *token;
//cur = (char*)buf;
//printk("ycx:lcd_reg_read_store buf =%s!\n",buf);
//token = strsep(&cur, ",");
//ret = sscanf(buf,"%c",&rw);
//
//if(ret!=1)
//{
//printk("ycx:lcd_reg_read_store ret =%d!\n",ret);
//return ret;
//}
//
//token = strsep(&cur,",");
//if(rw == 'r' || rw == 'R'){
//printk("ycx:lcd_reg_read_store read reg\n");
//memset(&read_table,0,sizeof(struct ddp_lcm_read_cmd_table));
//if(token == NULL)
//goto error;
//read_table.cmd = simple_strtol(token,NULL,0);
//token = strsep(&cur,",");
//if(token == NULL)
//goto error;
//read_table.count=simple_strtol(token,NULL,0);
//printk("read cmd 0x%0x,count %d!\n",read_table.cmd,read_table.count);
//do_lcm_vdo_lp_read(&read_table);
//}else if(rw == 'w' || rw == 'W'){
//struct ddp_lcm_write_cmd_table write_table;
//printk("ycx:lcd_reg_read_store write reg\n");
//memset(&write_table,0,sizeof(struct ddp_lcm_write_cmd_table));
//if(token == NULL)
//goto error;
//write_table.cmd = simple_strtol(token,NULL,0);
//token = strsep(&cur,",");
//while(token){
//write_table.para_list[i++]=simple_strtol(token,NULL,0);
//token = strsep(&cur,",");
//if(i>=64)
//{
//printk("count is too long\n");
//return -1;
//}
//}
//write_table.count = i;
//printk("ycx write cmd 0x%0x,count %d,para_list 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x!\n", write_table.para_list[0],write_table.para_list[1],write_table.para_list[2],write_table.para_list[3],write_table.para_list[4],write_table.para_list[5],write_table.para_list[6],write_table.para_list[7],write_table.para_list[8],write_table.para_list[9]);
//do_lcm_vdo_lp_write(&write_table);
//}else if(rw == 's' || rw == 'S'){
//printk("ycx start ESD: echo s/S > lcd_reg_read!\n");
//primary_display_esd_check_enable(1);
//}else if(rw == 'p' || rw == 'P'){
//printk("ycx pause ESD: echo p/P > lcd_reg_read!\n");
//primary_display_esd_check_enable(0);
//}else
//goto error;
//
//return n;
//
//error:
//printk("invalid data!\n");
//printk("please input:\n");
//printk("pause ESD: echo p/P > lcd_reg_read!\n");
//printk("read 2 value from reg 0x52:echo r ,0x52,2 > lcd_reg_read!\n");
//printk("write value 0xff,0xff to reg 0x51:echo w,0x51,0xff,0xff > lcd_reg_read\n");
//printk("start ESD: echo s/S > lcd_reg_read!\n");
//return -1;
//
//}

//static struct kobj_attribute lcd_read_reg_attr = {
//	.attr = {
//		.name = "lcd_read_reg",
//		.mode = 0644,
//	},
//	.show =&lcd_reg_read_show,
//	.store= &lcd_reg_read_store,
//};

//static ssize_t rpmb_key_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
//{
//	return sprintf(buf, "write 1 to reboot&bind rpmb\n");
//}
//static ssize_t rpmb_key_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
//{
//	kernel_restart("rpmbbind");
//    return n;
//}
//static struct kobj_attribute rpmb_key_attr = {
//	.attr = {
//		.name = "reboot2rpmbbind",
//		.mode = 0666,
//	},
//	.show =&rpmb_key_show,
//	.store= &rpmb_key_store,
//};

//extern int sec_schip_enabled(void);
//static ssize_t efuse_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
//{
//	return sprintf(buf, "%d\n", sec_schip_enabled()? 1:0);
//}
//
//static struct kobj_attribute sboot_efuse_info_attr = {
//	.attr = {
//		.name = "hw_efuse_info",
//		.mode = 0444,
//	},
//	.show =&efuse_info_show,
//};
// back_camera info start

static ssize_t cam_err_status_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

	if( 0 == (cam_err_code & 0x3FFFFFFF))
	{
		s += sprintf(s, "%s\n", "pass");
	}
	else
	{
		s += sprintf(s, "0x%x\n", (cam_err_code & 0x3FFFFFFF)); //MTK
	}

	return (s - buf);
}

static ssize_t cam_err_status_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	int ret = 0;
	u32 temp = 0x0;

	ret = kstrtouint(buf, 0, &temp);
	cam_err_code = temp & 0x3FFFFFFF;

	return n;
}

static struct kobj_attribute cam_err_status_attr = {
	.attr = {
		.name = "cam_err_status",
		.mode = 0644,
	},
	.show =&cam_err_status_show,
	.store= &cam_err_status_store,
};
static ssize_t back_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	if((back_camera_find_success>=sizeof(main_camera)/sizeof(char *))||(back_camera_find_success<=0)){
		s += sprintf(s, "%s\n",main_camera[0]);
	}else{
		int camid = 0;
		s += sprintf(s, "%d_%s-%s\n",camid, main_camera[back_camera_find_success], camera_status[back_camera_otp_status]);
	}

	return (s - buf);
}

static ssize_t back_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute back_camera_info_attr = {
	.attr = {
		.name = "back_camera",
		.mode = 0644,
	},
	.show =&back_camera_info_show,
	.store= &back_camera_info_store,
};
// back_camera info end

// back2_camera info start
static ssize_t back2_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	if((back2_camera_find_success>=sizeof(main2_camera)/sizeof(char *))||(back2_camera_find_success<=0)){
		s += sprintf(s, "%s\n",main2_camera[0]);
	}else{
		int camid = camera_back_probe_ok + camera_front_probe_ok;
		s += sprintf(s, "%d_%s-%s\n",camid, main2_camera[back2_camera_find_success], camera_status[back2_camera_otp_status]);
	}	

	return (s - buf);
}

static ssize_t back2_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute back2_camera_info_attr = {
	.attr = {
		.name = "back2_camera",
		.mode = 0644,
	},
	.show =&back2_camera_info_show,
	.store= &back2_camera_info_store,
};
// back2_camera info end

static ssize_t back3_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	if((back3_camera_find_success>=sizeof(main3_camera)/sizeof(char *))||(back3_camera_find_success<=0)){
		s += sprintf(s, "%s\n",main3_camera[0]);
	}else{
		int camid = camera_back_probe_ok + camera_front_probe_ok + camera_back2_probe_ok + camera_front2_probe_ok;
		s += sprintf(s, "%d_%s-%s\n",camid, main3_camera[back3_camera_find_success], camera_status[back3_camera_otp_status]);
	}	

	return (s - buf);
}

static ssize_t back3_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute back3_camera_info_attr = {
	.attr = {
		.name = "back3_camera",
		.mode = 0644,
	},
	.show =&back3_camera_info_show,
	.store= &back3_camera_info_store,
};

static ssize_t front_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

	if((front_camera_find_success>=sizeof(sub_front_camera)/sizeof(char *))||(front_camera_find_success<=0))
    {
		s += sprintf(s, "%s\n",sub_front_camera[0]);
	}
    else
    {
		int camid = camera_back_probe_ok;
		s += sprintf(s, "%d_%s-%s\n",camid, sub_front_camera[front_camera_find_success], camera_status[front_camera_otp_status]);
	}	
	
	return (s - buf);
}

static ssize_t front_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute front_camera_info_attr = {
	.attr = {
		.name = "front_camera",
		.mode = 0644,
	},
	.show =&front_camera_info_show,
	.store= &front_camera_info_store,
};

static ssize_t front2_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

	if((front2_camera_find_success>=sizeof(sub_front2_camera)/sizeof(char *))||(front2_camera_find_success<=0))
    {
		s += sprintf(s, "%s\n",sub_front2_camera[0]);
	}
    else
    {
		int camid = camera_back_probe_ok + camera_front_probe_ok + camera_back2_probe_ok;
		s += sprintf(s, "%d_%s-%s\n",camid, sub_front2_camera[front2_camera_find_success], camera_status[front2_camera_otp_status]);
	}	
	
	return (s - buf);
}

static ssize_t front2_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute front2_camera_info_attr = {
	.attr = {
		.name = "front2_camera",
		.mode = 0644,
	},
	.show =&front2_camera_info_show,
	.store= &front2_camera_info_store,
};

//songzhen DATA20200629 Romanee modify for camera msn begin

static ssize_t back_camera_msn_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
    return sprintf(buf, "%s\n", back_camera_msn);
}

static ssize_t back_camera_msn_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static struct kobj_attribute back_camera_msn_attr = {
	.attr = {
		.name = "back_camera_msn",
		.mode = 0644,
	},
	.show =&back_camera_msn_show,
	.store= &back_camera_msn_store,
};

static ssize_t front_camera_msn_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
    return sprintf(buf, "%s\n", front_camera_msn);
}

static ssize_t front_camera_msn_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static struct kobj_attribute front_camera_msn_attr = {
	.attr = {
		.name = "front_camera_msn",
		.mode = 0644,
	},
	.show =&front_camera_msn_show,
	.store= &front_camera_msn_store,
};

static ssize_t macro_camera_msn_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
    return sprintf(buf, "%s\n", macro_camera_msn);
}

static ssize_t macro_camera_msn_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static struct kobj_attribute macro_camera_msn_attr = {
	.attr = {
		.name = "macro_camera_msn",
		.mode = 0644,
	},
	.show =&macro_camera_msn_show,
	.store= &macro_camera_msn_store,
};
//songzhen DATA20200629 Romanee modify for camera msn end

//lishunyu add
char charge_ic_vendor_name[50]={0};
static ssize_t chargeric_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",&charge_ic_vendor_name[0]);
	return (s - buf);
}

static ssize_t chargeric_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute chargeric_info_attr = {
	.attr = {
		.name = "smb_info",
		.mode = 0644,
	},
	.show =&chargeric_info_show,
	.store= &chargeric_info_store,
};

//lishunyu add
extern char battery_vendor_name[50];
static ssize_t battery_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",&battery_vendor_name[0]);
	return (s - buf);
}

static ssize_t battery_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute battery_info_attr = {
	.attr = {
		.name = "bat_info",
		.mode = 0644,
	},
	.show =&battery_info_show,
	.store= &battery_info_store,
};

//lishunyu add
extern char typec_cc_direction[10];
static ssize_t typec_cc_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",&typec_cc_direction[0]);
	return (s - buf);
}

static ssize_t typec_cc_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute typec_cc_attr = {
	.attr = {
		.name = "typec_cc_dir",
		.mode = 0644,
	},
	.show =&typec_cc_show,
	.store= &typec_cc_store,
};

#if 0
static ssize_t torch_flash_onoff_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	return sprintf(buf, "%d\n", torch_flash_level); //? 1:0);
}
static ssize_t torch_flash_onoff_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{

    int res=0, temp_level=0;
    res = kstrtouint(buf, 10, &temp_level);

    if( (temp_level >= 0)&&(temp_level < 8) )
    {
        torch_flash_level = temp_level;
        ontim_torch_onoff(torch_flash_level);
    }
	return n;
}
static struct kobj_attribute touch_flash_onoff_info_attr = {
	.attr = {
		.name = "torch_flash",
		.mode = 0644,
	},
	.show =&torch_flash_onoff_info_show,
	.store= &torch_flash_onoff_info_store,
};
#endif

#if 0
static struct kobj_attribute modem_info_attr = {
	.attr = {
		.name = "modem_info",
		.mode = 0444,
	},
	.show = &modem_show,
};
#endif
//end
//#if 0
//static ssize_t lcd_driving_mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
//{
//       unsigned int val;
//       int res=0;
//	   
//	 res = kstrtouint(buf, 10, &val);
//
//	kernel_restart("nff_test");
//	//lcm_kernel_detect_drv->esd_check();
//	//if(lcd_detect_mipi_info.lcd_set_driving_mode)
//	//	lcd_detect_mipi_info.lcd_set_driving_mode(&lcd_detect_mipi_info,val);
//	//else
//		//printk(KERN_ERR "[kernel]:lcd_set_driving_mode not found!.\n");
//	return n;
//}
//
//static struct kobj_attribute lcd_driving_mode_set_attr = {
//	.attr = {
//		.name = "lcd_driving_mode_set_info",
//		.mode = 0644,
//	},
//	.store = &lcd_driving_mode_store,
//};
//#endif
#if 1
static ssize_t i2c_devices_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
//	u8 string[5]={'\0'};
	int tmp=0;
	tmp|= (tp_probe_ok<<0);
	tmp|= (camera_front_probe_ok<<1);
	tmp|= (camera_back_probe_ok<<2);
	tmp|= (gsensor_probe_ok<<3);
	tmp|= (proximity_probe_ok<<4);
	tmp|= (charger_probe_ok<<5);
	tmp|= (pmu_probe_ok<<6);
	tmp|= (compass_probe_ok<<7);
	tmp|= (sar_probe_ok<<8);
tmp|= (camera_back2_probe_ok<<9);
tmp|= (camera_back3_probe_ok<<10);
tmp|= (camera_front2_probe_ok<<11);
	tmp|= (fingerprint_probe_ok<<31);

	//itoa((int)tmp,string);
	s += sprintf(s, "0x%x\n",tmp);
	
	return (s - buf);
}
static struct kobj_attribute i2c_devices_info_attr = {
	.attr = {
		.name = "i2c_devices_probe_info",
		.mode = 0444,
	},
	.show =&i2c_devices_info_show,
};
#endif
//+add by hzb

#if 0
#if  defined (CONFIG_ARM64)  //Titan_TL PRJ
int get_pa_num(void)
{
     //add by pare for modem gpio check
#define GPIO_VERSION_PIN1    (GPIO95 | 0x80000000)
#define GPIO_VERSION_PIN2    (GPIO96 | 0x80000000)
#define GPIO_VERSION_PIN3    (GPIO93 | 0x80000000)
#define GPIO_VERSION_PIN4    (GPIO94 | 0x80000000)
	
	int pin1_val = 0, pin2_val = 0, pin3_val = 0, pin4_val = 0;

      mt_set_gpio_pull_select(GPIO_VERSION_PIN1, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN1, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN1, GPIO_DIR_IN);

      mt_set_gpio_pull_select(GPIO_VERSION_PIN2, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN2, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN2, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN2, GPIO_DIR_IN);

      mt_set_gpio_pull_select(GPIO_VERSION_PIN3, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN3, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN3, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN3, GPIO_DIR_IN);

      mt_set_gpio_pull_select(GPIO_VERSION_PIN4, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN4, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN4, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN4, GPIO_DIR_IN);

	mdelay(20);

	pin1_val = mt_get_gpio_in(GPIO_VERSION_PIN1);
	pin2_val = mt_get_gpio_in(GPIO_VERSION_PIN2);
	pin3_val = mt_get_gpio_in(GPIO_VERSION_PIN3);
	pin4_val = mt_get_gpio_in(GPIO_VERSION_PIN4);
	
	printk(KERN_ERR "%s:  pin1 is %d, pin2 is %d, pin3 is %d, pin4 is %d\n",__func__, pin1_val, pin2_val,pin3_val,pin4_val);
	
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_DISABLE);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN2, GPIO_PULL_DISABLE);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN3, GPIO_PULL_DISABLE);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN4, GPIO_PULL_DISABLE);


#if  !defined (CONFIG_MTK_C2K_SUPPORT)  //Titan_TL
     if (pin1_val && pin4_val ) 
     {
         return 1;
     }
     else 
     {
         return 0;
     }
#else
     if (0) //( pin2_val && ( !pin3_val) && ( !pin4_val))
     {
         return 1;
     }
     else 
     {
         return 0;
     }
#endif
}
#else
int get_pa_num(void)
{
//baixue add for disable second PA
      return 0;

     //add by pare for modem gpio check
#define GPIO_VERSION_PIN1    (GPIO96 | 0x80000000)
	
	int pin1_val = 0, pin2_val = 0;

      mt_set_gpio_pull_select(GPIO_VERSION_PIN1, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN1, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN1, GPIO_DIR_IN);

	mdelay(10);
	
	pin1_val = mt_get_gpio_in(GPIO_VERSION_PIN1);
	
	printk(KERN_ERR "%s:  pin1 is %d\n",__func__, pin1_val);
	
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_DISABLE);


      return !pin1_val;
	
}
#endif

static ssize_t RF_PA_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
//	u8 string[5]={'\0'};
	int tmp=0;
	
	tmp = get_pa_num();

	s += sprintf(s, "%d\n",tmp);
	
	return (s - buf);
}

static struct kobj_attribute RF_PA_info_attr = {
	.attr = {
		.name = "RF_PA_Type",
		.mode = 0444,
	},
	.show =&RF_PA_info_show,
};
#endif
//+add by liujingchuan for check cust prj ver 
#include <linux/gpio.h>
int get_hw_prj(void)
{
       unsigned int gpio_base =343;

	unsigned int pin0=93;
	unsigned int pin1=92;

	int pin_val = 0;
	int hw_prj=0;

	
	pin_val =    gpio_get_value(gpio_base+pin0) & 0x01;
	pin_val |= (gpio_get_value(gpio_base+pin1) & 0x01) << 1;
	hw_prj = pin_val;
	
	printk(KERN_ERR "%s: hw_prj is %x ;\n",__func__, hw_prj);

	return  hw_prj;
	
}
static ssize_t get_hw_prj_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

      s += sprintf(s, "0x%02x\n",get_hw_prj());
	
	return (s - buf);
	
}

static struct kobj_attribute get_hw_prj_attr = {
	.attr = {
		.name = "hw_prj",
		.mode = 0644,
	},
	.show =&get_hw_prj_show,
};

//-add by liujingchuan for check cust prj ver 
int get_hw_ver_info(void)
{ 
       unsigned int gpio_base =343;

	unsigned int pin0=121;
	unsigned int pin1=54;
	unsigned int pin2=53;
	unsigned int pin3=5;
	unsigned int pin4=11;
	int pin_val = 0;
	int hw_ver=0;

	
	pin_val =    gpio_get_value(gpio_base+pin0) & 0x01;
	pin_val |= (gpio_get_value(gpio_base+pin1) & 0x01) << 1;
	pin_val |= (gpio_get_value(gpio_base+pin2) & 0x01) << 2;
	pin_val |= (gpio_get_value(gpio_base+pin3) & 0x01) << 3;
	pin_val |= (gpio_get_value(gpio_base+pin4) & 0x01) << 4;
	hw_ver = pin_val;
	
	printk(KERN_ERR "%s: hw_ver is %x ;\n",__func__, hw_ver);

	return  hw_ver;

}
static ssize_t get_hw_ver_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

      s += sprintf(s, "0x%02x\n",get_hw_ver_info());
	
	return (s - buf);
}

static struct kobj_attribute get_hw_ver_attr = {
	.attr = {
		.name = "hw_ver",
		.mode = 0644,
	},
	.show =&get_hw_ver_show,
};
////-add by hzb

//add by chenyu for sd status
int get_sd_status_info(void)
{ 
	unsigned int cd_gpios =50;
	int pin_val = 0;
	
	pin_val = gpio_get_value(cd_gpios);
	
	printk(KERN_ERR "%s: hw_sd_tray is %d ;\n",__func__, pin_val);

	return  pin_val;

}
static ssize_t get_sd_status_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

	s += sprintf(s, "%d\n",get_sd_status_info());
	
	return (s - buf);
}

static struct kobj_attribute get_sd_status_attr = {
	.attr = {
		.name = "hw_sd_tray",
		.mode = 0644,
	},
	.show =&get_sd_status_show,
};

static void check_cust_ver(void){
//    int rb_flag = 0;;
//    struct hw_ver * hw_ver_info=get_hw_ver_info();
//    if ( hw_ver_info == NULL )
//    {    
//    	printk(KERN_ERR "%s: Get PRJ info Error!!!\n",__func__);
//        return;
//    }
//    printk(KERN_ERR "%s: Build PRJ is %s, This PRJ is %s!!\n",__func__, PRJ_NAME, hw_ver_info->name);
//    if((!strcmp(hw_ver_info->name, PRJ_NAME)) || (!strcmp(hw_ver_info->name, "NULL"))){
//    	printk(KERN_ERR "%s: Version Pass!!\n",__func__);
//    }
//    else
//    {      
//        printk(KERN_ERR "%s: Version Error!!!\n",__func__);
//       // kernel_restart("prjerr");
//    }
}
//-add by hzb for check cust prj ver 

static struct attribute * g[] = {
	&get_hw_prj_attr.attr,//add by liujingchuan
	&get_hw_ver_attr.attr,//+add by hzb
//	&get_equip_attr.attr,
	&lcm_qr_info_attr.attr,
	&lcd_info_attr.attr,//+add by liuwei
	&touch_version_attr.attr,//+add by fanjie
	&touch_info_attr.attr,//+add by fanjie
//  &lcd_read_reg_attr.attr,//+add by yangcaixia
	&get_sd_status_attr.attr,//+add by chenyu
//	&rpmb_key_attr.attr,//+add by yzw
    //&modem_info_attr.attr,//+add by youjiangong
	//&lcd_driving_mode_set_attr.attr,//+add by liuwei
	&i2c_devices_info_attr.attr,//+add by liuwei
	&back_camera_info_attr.attr,
	&back2_camera_info_attr.attr,
	&back3_camera_info_attr.attr,

    &cam_err_status_attr.attr,
	&front_camera_info_attr.attr,
//songzhen DATA20200629 Romanee modify for camera msn begin
	&back_camera_msn_attr.attr,
	&front_camera_msn_attr.attr,
	&macro_camera_msn_attr.attr,
//songzhen DATA20200629 Romanee modify for camera msn end
//	&front2_camera_info_attr.attr,
	//&touch_flash_onoff_info_attr.attr,
	//&enemmd_attr.attr,//add by youjiangong
	//&RF_PA_info_attr.attr,    //add by hzb
//	&sboot_efuse_info_attr.attr,//add by yzw
	&chargeric_info_attr.attr, //lilshunyu add 
	&battery_info_attr.attr,   //lishunyu add
	&typec_cc_attr.attr,   //lishunyu add
	&fp_info_attr.attr, //add by shangfei
	&fp_id_attr.attr, //add by yaolihui
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};
#if 0
int touchscreen_has_steel_film=0;
static int __init touchscreen_film_setup(char *str)
{
	int en;
	if(!get_option(&str, &en))
		return 0;
	touchscreen_has_steel_film = en;
	return 1;
}

__setup("tp_film=", touchscreen_film_setup);

int get_touchscreen_film_state(void)
{
	printk("[kernel]:touchscreen_has_steel_film=%d.\n",touchscreen_has_steel_film);
	return touchscreen_has_steel_film;
}
#endif
#if 0
int lcm_id=0x83;
static int __init lcm_id_setup(char *str)
{
        int en;
        if(!get_option(&str, &en))
                return 0;
        lcm_id = en;
        return 1;
}
int get_lcm_id(void)
{
        printk("[kernel]:get_lcm_id=%x.\n",lcm_id);
        return lcm_id;
}
__setup("lcm_id=", lcm_id_setup);
#endif
static int __init bootinfo_init(void)
{
	int ret = -ENOMEM;
	
	//printk("%s,line=%d\n",__func__,__LINE__);  

	bootinfo_kobj = kobject_create_and_add("ontim_bootinfo", NULL);

	if (bootinfo_kobj == NULL) {
		printk("bootinfo_init: kobject_create_and_add failed\n");
		goto fail;
	}

	ret = sysfs_create_group(bootinfo_kobj, &attr_group);
	if (ret) {
		printk("bootinfo_init: sysfs_create_group failed\n");
		goto sys_fail;
	}
    
	return ret;
sys_fail:
	kobject_del(bootinfo_kobj);
fail:
	return ret;

}

static int __init prjinfo_init(void)
{
    printk("bootinfo_init: check_hw_ver Start!\n");
    check_cust_ver();
    return 0;
}

static void __exit bootinfo_exit(void)
{

	if (bootinfo_kobj) {
		sysfs_remove_group(bootinfo_kobj, &attr_group);
		kobject_del(bootinfo_kobj);
	}
}

arch_initcall(bootinfo_init);
device_initcall(prjinfo_init);
module_exit(bootinfo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Boot information collector");
