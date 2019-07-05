#include <string.h>
#include "ctl_common.h"
#include "ctl_icm_flash.h"
#include "drv_xt25q32.h"
#include "ctl_fds_log.h"
#include "ctl_icm20602_queue.h"
#include "ctl_ble.h"
#include "ctl_led.h"

typedef struct {
	bool icm_data_read_flag;
	uint32_t read_count;
	uint32_t send_count;
}icm_data_read_t;

static icm_data_read_t g_icm_data_read = 
{
	.icm_data_read_flag =  false,
	.read_count = 0,
	.send_count = 0,	
};


//#define ICM_DATA_HEAD_START_ADDR	(XT25Q_CHIP_SIZE-XT25Q_SECTOR_SIZE)
#define ICM_DATA_START_ADDR			0
#define ICM_DATA_END_ADDR			XT25Q_CHIP_SIZE//(XT25Q_CHIP_SIZE-XT25Q_SECTOR_SIZE)

typedef __packed struct {
	uint32_t first_index;
	uint32_t last_index;
	uint32_t erased_sector_index;
	uint32_t over_write;
}flash_icm_data_head_t;

static flash_icm_data_head_t g_icm_data_head = {
	.first_index 		= 0,
	.last_index			= 0,
	.erased_sector_index= 0,
	.over_write			= 0,
};


static ret_code_t ctl_flash_icm_data_head_read(flash_icm_data_head_t *icm_data_head)
{
#ifdef FDS_HEAD
	uint8_array_t flex_log;

	VERIFY_PARAM_NOT_NULL(icm_data_head);

	flex_log.p_data = (data_t *)icm_data_head;
	flex_log.size   = sizeof(flash_icm_data_head_t);

	return flex_log_read_hex(FLEX_LOG_ICM_DATA_HEAD_ID, 0, &flex_log);
#else
	uint8_t data[16];
	/*
	drv_xt25q_buf_read(ICM_DATA_HEAD_START_ADDR,(uint8_t *)icm_data_head,sizeof(flash_icm_data_head_t));
	if(icm_data_head->last_index == 0xFFFFFFFF)
		return 1;
	else
		return 0;*/
	flex_debug("\r\n");
	for(uint8_t i=0; i<sizeof(flash_icm_data_head_t); i++)
	{
		data[i] = *(uint32_t *)(0x5c000+i);
		flex_debug("%x",data[i]);
	}
	memcpy(icm_data_head,data,sizeof(flash_icm_data_head_t));
	if(icm_data_head->last_index == 0xFFFFFFFF)
		return 1;
	else
		return 0;
#endif
}

static ret_code_t ctl_flash_icm_data_head_write(flash_icm_data_head_t icm_data_head)
{
#ifdef FDS_HEAD
	uint8_array_t 		flex_log;
	ret_code_t			err_code;

	VERIFY_PARAM_NOT_NULL(&icm_data_head);

	flex_log.p_data = (data_t *)&icm_data_head;
	flex_log.size   = sizeof(flash_icm_data_head_t);

	err_code = flex_log_save_hex(FLEX_LOG_ICM_DATA_HEAD_ID, 0, &flex_log);
	VERIFY_SUCCESS(err_code);
#else
	//368
	//return sd_flash_write((uint32_t * const)0x5c000,(uint32_t const * const)&icm_data_head,sizeof(flash_icm_data_head_t));
	//drv_xt25q_buf_write(ICM_DATA_HEAD_START_ADDR,(uint8_t *)&icm_data_head,sizeof(flash_icm_data_head_t));
	//return 0;
#endif
	return 0;
}
#if 0
static ret_code_t ctl_flash_icm_data_head_clear(void)
{
	return sd_flash_page_erase(92);
	//drv_xt25q_sector_erase(ICM_DATA_HEAD_START_ADDR);
}
#endif
void ctl_icm_data_read_set(uint32_t count)
{
	g_icm_data_read.icm_data_read_flag = true;
	g_icm_data_read.read_count = count;
}



void ctl_flash_icm_data_write(uint8_t *buf, uint32_t len)
{
	uint32_t erased_addr	= 0;
	uint32_t start_addr 	= 0;
	uint32_t end_addr 		= 0;	
	
	
	//if(g_icm_data_head.last_index==0)
	//	flex_debug("\r\n time: %d",rtc_timestamp_get());
	//print_hex(buf, len);
	//print_hex((uint8_t *)&g_icm_data_head, sizeof(flash_icm_data_head_t));
	erased_addr	= g_icm_data_head.erased_sector_index*XT25Q_SECTOR_SIZE;
	start_addr	= g_icm_data_head.last_index*sizeof(icm_data_t);
	end_addr	= start_addr+len;

	if(end_addr>=ICM_DATA_END_ADDR)
	{
		erased_addr = 0;
		start_addr	= 0;
		end_addr	= len;
		g_icm_data_head.over_write += 1;
	}
	while(erased_addr<end_addr)
	{
		drv_xt25q_sector_erase(erased_addr);
		erased_addr += XT25Q_SECTOR_SIZE;
	}
	drv_xt25q_buf_write(start_addr,buf,len);
	
	g_icm_data_head.erased_sector_index = erased_addr/XT25Q_SECTOR_SIZE;
	g_icm_data_head.last_index	= end_addr/sizeof(icm_data_t);
	if(g_icm_data_head.over_write>0)
			g_icm_data_head.first_index = erased_addr/sizeof(icm_data_t);
	//ctl_flash_icm_data_head_clear();
	//ctl_flash_icm_data_head_write(g_icm_data_head);
	/*if(g_icm_data_head.last_index==5000) //&&g_icm_data_head.last_index<=5016)
	{
		flex_debug("\r\n time: %d",rtc_timestamp_get());
	}*/
}

static void ctl_icm_data_send(icm_data_t data)
{
	if(g_icm_data_read.icm_data_read_flag)
	{
		if(g_icm_data_read.send_count<g_icm_data_read.read_count)
		{
			multip_data_send((uint8_t *)&data,sizeof(icm_data_t));
			g_icm_data_read.send_count++;	
			//flex_debug("\r\n ms:%d",data.ms);			
		}
		else
		{
			flex_debug("\r\n send count:%d",g_icm_data_read.send_count);
			memset(&g_icm_data_read,0,sizeof(icm_data_read_t));
			ctl_led_status_set(LED_CONNECT_STATUS);
		}
	}
}
void ctl_flash_icm_storage(void)
{
	icm_data_t data;
	
	//flex_debug("\r\n count:%d",count++);
	queue_data_get(&data);
	ctl_flash_icm_data_write((uint8_t *)&data,sizeof(icm_data_t));
	ctl_icm_data_send(data);
}

void ctl_flash_icm_data_read(uint32_t count)
{
	int32_t total_size = 0;
	int32_t first_index = 0;
	icm_data_t icm_data;
	
	//flex_debug("\r\n ctl_flash_icm_data_read");
	//print_hex((uint8_t *)&g_icm_data_head, sizeof(flash_icm_data_head_t));
	total_size = g_icm_data_head.last_index - g_icm_data_head.first_index;
	if(total_size<0)
	{
		total_size += ICM_DATA_END_ADDR/sizeof(icm_data_t);
	}
	if(count>total_size)
		count = total_size;
	first_index = g_icm_data_head.last_index - count;
	if(first_index<0)
	{
		first_index += ICM_DATA_END_ADDR/sizeof(icm_data_t);
	}
	for(uint32_t i=first_index;i!=g_icm_data_head.last_index;i++)
	{
		if(i==ICM_DATA_END_ADDR/sizeof(icm_data_t))
		{
			i = 0;
			if(i==g_icm_data_head.last_index)
				break;
		}
		drv_xt25q_buf_read(i*sizeof(icm_data_t),(uint8_t *)&icm_data,sizeof(icm_data_t));
		print_hex((uint8_t *)&icm_data,sizeof(icm_data_t));
	}
}

void ctl_flash_icm_data_read_last(icm_data_t *icm_data)
{
	uint32_t addr=0;
	addr = (g_icm_data_head.last_index-1)*sizeof(icm_data_t);
	drv_xt25q_buf_read(addr,(uint8_t *)icm_data,sizeof(icm_data_t));
}
void ctl_flash_icm_data_read_from_0(uint32_t count)
{
	icm_data_t icm_data;
	for(uint32_t i=0;i<count;i++)
	{
		drv_xt25q_buf_read(i*sizeof(icm_data_t),(uint8_t *)&icm_data,sizeof(icm_data_t));
		//print_hex((uint8_t *)&icm_data,sizeof(icm_data_t));
	}
}

uint32_t ctl_flash_icm_got_last_index(void)
{
	return g_icm_data_head.last_index;
}
void ctl_flash_icm_data_init(void)
{
	ret_code_t error = 0;
	
	//drv_fds_flash_init();
	nrf_delay_ms(10);

	//memset(&g_icm_data_head,1,sizeof(flash_icm_data_head_t));
	//error = ctl_flash_icm_data_head_write(g_icm_data_head);
	//flex_debug("\r\n w error:%d",error);
	
	error = ctl_flash_icm_data_head_read(&g_icm_data_head);
	if(error)
	{
		memset(&g_icm_data_head,0,sizeof(flash_icm_data_head_t));
		ctl_flash_icm_data_head_write(g_icm_data_head);
	}
	nrf_delay_ms(10);
	memset(&g_icm_data_head,0,sizeof(flash_icm_data_head_t));
	error = ctl_flash_icm_data_head_write(g_icm_data_head);
	flex_debug("\r\n w error:%d",error);
	error = ctl_flash_icm_data_head_read(&g_icm_data_head);
	print_hex((uint8_t *)&g_icm_data_head, sizeof(flash_icm_data_head_t));
	flex_debug("\r\n r error:%d",error);
#if 0
	flex_debug("\r\n error:%d",error);
	#ifdef FDS_HEAD	
	error = flex_log_delete_id(FLEX_LOG_ICM_DATA_HEAD_ID);
	#else
	ctl_flash_icm_data_head_clear();
	#endif
    //VERIFY_SUCCESS(error);
	memset(&g_icm_data_head,0,sizeof(flash_icm_data_head_t));
	ctl_flash_icm_data_head_write(g_icm_data_head);
	drv_xt25q_sector_erase(50*XT25Q_SECTOR_SIZE);
	error = ctl_flash_icm_data_head_read(&g_icm_data_head);
	
	print_hex((uint8_t *)&g_icm_data_head, sizeof(flash_icm_data_head_t));
#endif
}

