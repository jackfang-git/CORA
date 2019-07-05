#include <string.h>
#include <stdbool.h>
#include "ctl_icm20602_queue.h"
#include "ctl_common.h"
#include "drv_rtc.h"
#include "ctl_icm_flash.h"

static icm_data_t g_rcbuf[QUEUE_BUF_SIZE];
static uint8_t g_rcbuf_first_index = 0;
static uint8_t g_rcbuf_last_index = 0;
//static bool g_queue_new_data_flag = 0;



ret_code_t queue_icm_data_got(void)
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
	//if(quene_get_last_index()==0)
	//	ctl_flash_icm_data_write((uint8_t *)g_rcbuf,sizeof(icm_data_t)*QUEUE_BUF_SIZE);
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

void queue_data_put(icm_data_t data)
{
	uint8_t next_index;
	//g_queue_new_data_flag = true;
	next_index = g_rcbuf_last_index;
	next_index++;
	if(next_index>=QUEUE_BUF_SIZE)
	{
		next_index = 0;
	}
	if(next_index==g_rcbuf_first_index)
	{
		static uint32_t count = 0;
		
		//flex_debug("\r\n************icm queue is full :%d*************\r\n",count++);
		return;
		//g_rcbuf_first_index++;
	}
	memcpy(&g_rcbuf[g_rcbuf_last_index],&data,sizeof(icm_data_t));
	g_rcbuf_last_index = next_index;
	//print_hex((uint8_t *)&data,sizeof(icm20602_data_t));
}

void queue_data_get(icm_data_t *data)
{
	uint8_t first_index = 0;
	
	//while(g_rcbuf_first_index == g_rcbuf_last_index);
	if(g_rcbuf_first_index == g_rcbuf_last_index)
	{
		static uint32_t count = 0;
		flex_debug("\r\n************icm queue is empty :%d*************\r\n",count++);
	}
	
	first_index = g_rcbuf_first_index;
	first_index++;
	if(first_index>=QUEUE_BUF_SIZE)
	{
		first_index = 0;
	}
	memcpy((uint8_t *)data,&g_rcbuf[g_rcbuf_first_index],sizeof(icm_data_t));
	g_rcbuf_first_index = first_index;
}

uint8_t queue_data_rebuf_size(void)
{
	int8_t size;
	size = g_rcbuf_last_index-g_rcbuf_first_index;
	if(size < 0)
	{
		size += QUEUE_BUF_SIZE;
	}
	return size;
}

void queue_data_get_last(icm_data_t *data)
{
	uint8_t cur_index;
	
	if(g_rcbuf_last_index==0)
		cur_index = QUEUE_BUF_SIZE-1;
	else
		cur_index = g_rcbuf_last_index-1;
	memcpy((uint8_t *)data,&g_rcbuf[cur_index],sizeof(icm_data_t));
}

void queue_data_get_req(uint8_t req_index,icm_data_t *data)
{
	memcpy((uint8_t *)data,&g_rcbuf[req_index],sizeof(icm_data_t));
}

uint8_t quene_get_last_index(void)
{
	return g_rcbuf_last_index;
}

uint8_t quene_get_first_index(void)
{
	return g_rcbuf_first_index;
}
void ctl_queue_data_init(void)
{
	memset(g_rcbuf,0,sizeof(g_rcbuf));
}

