#include <string.h>
#include "ctl_icm20602_timer.h"
#include "ctl_icm20602_queue.h"
#include "drv_icm20602.h"
#include "drv_rtc.h"
#include "flex_debug.h"
#include "ctl_cmd_queue.h"
#include "ctl_icm_flash.h"

#define TIMER_ICM20602_TIMEOUT		5

static uint8_t timer_icm20602_flag = 0;
static void timer_icm20602_handler(void);
static flex_rtc_timer_t timer_icm20602={RTC_CC_CHANNEL_0,TIMER_ICM20602_TIMEOUT,timer_icm20602_handler};

#if 0
static ret_code_t icm_data_got(void)
{
	
	icm_data_t read_buf;
	static uint32_t cmd_count = 0;
	if((cmd_count++) == 5000)
		flex_debug("\r\n cmd time: %d",rtc_timestamp_get());
	memset(&read_buf,0,sizeof(icm_data_t));
	ICM_Get_Gyroscope((short *)&read_buf.icm20602_data.gx,(short *)&read_buf.icm20602_data.gy,(short *)&read_buf.icm20602_data.gz);
	ICM_Get_Accelerometer((short *)&read_buf.icm20602_data.ax,(short *)&read_buf.icm20602_data.ay,(short *)&read_buf.icm20602_data.az);
	read_buf.ms = rtc_timestamp_get_ms();
	queue_data_put(read_buf);
	ctl_flash_icm_data_write((uint8_t *)&read_buf,sizeof(icm_data_t));
	/*
	if(quene_get_last_index()==0)
	{
		for(uint8_t index=0; index<QUEUE_BUF_SIZE; index++)
		{
			queue_data_get_req(index,&read_buf);
			ctl_flash_icm_data_write((uint8_t *)&read_buf,sizeof(icm_data_t));
		}
	}
	*/
	//start_icm20602_timer();
	return NRF_SUCCESS;
}
#endif
static void timer_icm20602_handler(void)
{
	
	static uint32_t count_test=0;
	timer_icm20602_flag = 0;
	
	
#if 0
	cmd_t cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.cid   		= CMD_ICM_DATA_GOT;
	cmd.param_len 	= 0;	
	cmd.cmd_type    = CMD_PERM_SYS;
	ctl_cmd_enqueue(&cmd);
	start_icm20602_timer();
#endif
	//queue_icm_data_got();
	
	//cmd_icm_data_got();
	//
	
	icm_data_t read_buf;
	memset(&read_buf,0,sizeof(icm_data_t));
	read_buf.ms = rtc_timestamp_get_ms();
	ICM_Get_Gyroscope((short *)&read_buf.icm20602_data.gx,(short *)&read_buf.icm20602_data.gy,(short *)&read_buf.icm20602_data.gz);
	ICM_Get_Accelerometer((short *)&read_buf.icm20602_data.ax,(short *)&read_buf.icm20602_data.ay,(short *)&read_buf.icm20602_data.az);
	//print_hex((uint8_t *)&read_buf,sizeof(icm20602_data_t));
	//read_buf.gx = test++;
	queue_data_put(read_buf);
	if((count_test++)==1000)
		flex_debug("\r\n count time: %d",rtc_timestamp_get());
	start_icm20602_timer();
}

void stop_icm20602_timer(void)
{
	if(timer_icm20602_flag==1)
		rtc_timer_stop(&timer_icm20602);
	timer_icm20602_flag = 0;
}

void start_icm20602_timer(void)
{
	if(timer_icm20602_flag==0)
		rtc_timer_start(&timer_icm20602);
	else
	{
		stop_icm20602_timer();
	}
	timer_icm20602_flag = 1;
}

void ctl_icm20602_timer_init(void)
{
	ctl_queue_data_init();
	start_icm20602_timer();
}
