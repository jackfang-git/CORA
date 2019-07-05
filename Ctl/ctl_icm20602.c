#include <string.h>
#include "ctl_icm20602.h"
#include "drv_icm20602.h"
#include "drv_rtc.h"
#include "ctl_ble.h"
#include "ctl_common.h"
#include "ctl_icm20602_queue.h"
#include "ctl_icm_flash.h"
#include "ctl_cmd_queue.h"


#define TIMER_ICM20602_READ_TIMEOUT		500
#define TIME_FACTOR						100

static uint32_t g_icm20602_read_timeout = TIMER_ICM20602_READ_TIMEOUT;

static uint8_t timer_icm20602_read_flag = 0;
static void timer_icm20602_read_handler(void);
static flex_rtc_timer_t timer_icm20602_read={RTC_CC_CHANNEL_0,TIMER_ICM20602_READ_TIMEOUT,timer_icm20602_read_handler};

static void stop_icm20602_read_timer(void);
static void start_icm20602_read_timer(void);



static void timer_icm20602_read_handler(void)
{
	cmd_t cmd;
	
	timer_icm20602_read_flag = 0;	
	memset(&cmd, 0, sizeof(cmd));
	cmd.cid   		= CMD_ICM_NOTIF;
	cmd.param_len 	= 0;	
	cmd.cmd_type    = CMD_PERM_SYS;
	ctl_cmd_enqueue(&cmd);
	/*
	icm_data_t read_buf;
	
	timer_icm20602_read_flag = 0;	
	memset(&read_buf,0,sizeof(icm20602_data_t));	
	ctl_flash_icm_data_read_last(&read_buf);

	print_hex((uint8_t *)&read_buf,sizeof(icm20602_data_t));
	data_send((uint8_t *)&read_buf,sizeof(icm_data_t));	*/
	start_icm20602_read_timer();
}

static void stop_icm20602_read_timer(void)
{
	if(timer_icm20602_read_flag==1)
		rtc_timer_stop(&timer_icm20602_read);
	timer_icm20602_read_flag = 0;
}

static void start_icm20602_read_timer(void)
{
	timer_icm20602_read.msec = g_icm20602_read_timeout;
	if(timer_icm20602_read_flag==0)
		rtc_timer_start(&timer_icm20602_read);
	else
	{
		stop_icm20602_read_timer();
	}
	timer_icm20602_read_flag = 1;
}

void ctl_icm20602_read_cycle(uint32_t data)
{
	g_icm20602_read_timeout = data*TIME_FACTOR; //ms
}


void ctl_icm20602_read_enable(void)
{
	start_icm20602_read_timer();
}
void ctl_icm20602_read_disable(void)
{
	stop_icm20602_read_timer();
}



