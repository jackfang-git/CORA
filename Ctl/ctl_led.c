#include <string.h>
#include "ctl_led.h"
#include "nrf_gpio.h"
#include "drv_rtc.h"
#include "nrf_delay.h"
#include "ctl_common.h"

#define	ONLY_RED_LED

#define LED_G_PIN	31
#define LED_R_PIN	30

#define POWER_ON_LONG_TIME	2000

#define LED_INIT()		{\
	nrf_gpio_cfg_output(LED_R_PIN);\
	nrf_gpio_cfg_output(LED_G_PIN);\
}

#define LED_R_ON()			nrf_gpio_pin_write(LED_R_PIN,1);
#define LED_R_OFF()			nrf_gpio_pin_write(LED_R_PIN,0);

#define LED_G_ON()			nrf_gpio_pin_write(LED_G_PIN,1);
#define LED_G_OFF()			nrf_gpio_pin_write(LED_G_PIN,0);

#define LED_ALL_ON()		{\
	nrf_gpio_pin_write(LED_R_PIN,1);\
	nrf_gpio_pin_write(LED_G_PIN,1);\
}
#define LED_ALL_OFF()		{\
	nrf_gpio_pin_write(LED_R_PIN,0);\
	nrf_gpio_pin_write(LED_G_PIN,0);\
}

#define LED_INVERT(LED_PIN)		nrf_gpio_pin_toggle(LED_PIN);


static led_contorl_t	g_led_contorl;

#ifndef ONLY_RED_LED
void ctl_led_status_handle(void)
{
	uint32_t cur_ms = 0;
	switch(g_led_contorl.led_status)
	{
		case LED_OFF_STATUS:
			break;
		case LED_POWER_ON_STATUS:
		{
			cur_ms = rtc_timestamp_get_ms();	
			if(g_led_contorl.sub_status==0)
			{
				LED_G_ON();
				LED_R_OFF();
				g_led_contorl.sub_status=1;
			}
			else if(g_led_contorl.sub_status==1)
			{
				if(cur_ms-g_led_contorl.start_time>POWER_ON_LONG_TIME)
				{
					LED_G_OFF();
					LED_R_ON();
					g_led_contorl.led_ope_time = cur_ms;
					g_led_contorl.sub_status=2;
				}
			}
			else
			{
				if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
				{
					LED_INVERT(LED_G_PIN);
					LED_INVERT(LED_R_PIN);
					g_led_contorl.led_ope_time = cur_ms;
				}
			}			
		}
			break;
		case LED_ADV_STATUS:
		{
			cur_ms = rtc_timestamp_get_ms();
			if(cur_ms-g_led_contorl.start_time>g_led_contorl.duration)
			{
				LED_R_OFF();
			}
			else
			{
				if(!g_led_contorl.led_ope_time)
				{
					LED_R_ON();
					g_led_contorl.led_ope_time = cur_ms;
				}
				else
				{
					if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
					{
						LED_INVERT(LED_R_PIN);
						g_led_contorl.led_ope_time = cur_ms;
					}
				}
			}
		}
			break;
		case LED_CONNECT_STATUS:
		case LED_SEND_DATA_STATUS:
		{
			cur_ms = rtc_timestamp_get_ms();
			{
				if(!g_led_contorl.led_ope_time)
				{
					LED_G_ON();
					LED_R_OFF();
					g_led_contorl.led_ope_time = cur_ms;
				}
				else
				{
					if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
					{
						LED_INVERT(LED_G_PIN);
						g_led_contorl.led_ope_time = cur_ms;
					}
				}
			}
		}
			break;
		case LED_BATT_WARNING:
		{
			cur_ms = rtc_timestamp_get_ms();
			{
				if(!g_led_contorl.led_ope_time)
				{
					LED_R_ON();
					LED_G_OFF();
					g_led_contorl.led_ope_time = cur_ms;
				}
				else
				{
					if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
					{
						LED_INVERT(LED_R_PIN);
						g_led_contorl.led_ope_time = cur_ms;
					}
				}
			}
		}
			break;
		default:
			break;
	}
}

void ctl_led_status_set(led_status_t led_status)
{
	switch (led_status)
	{
		case LED_POWER_ON_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 1000;
			break;
		case LED_ADV_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.duration		= 5*1000;
			g_led_contorl.interval_time	= 500;
			break;
		case LED_CONNECT_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 800;
			break;
		case LED_SEND_DATA_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 200;
			break;
		case LED_BATT_WARNING:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 200;
			break;
		default:
			break;
	}
}

void ctl_led_init(void)
{
	LED_INIT();
	LED_R_OFF();
	LED_G_ON();
}

#else
void ctl_led_status_handle(void)
{
	uint32_t cur_ms = 0;
	switch(g_led_contorl.led_status)
	{
		case LED_OFF_STATUS:
			break;
		case LED_POWER_ON_STATUS:
		{
			cur_ms = rtc_timestamp_get_ms();	
			if(g_led_contorl.sub_status==0)
			{
				LED_R_ON();
				g_led_contorl.sub_status=1;
			}
			else if(g_led_contorl.sub_status==1)
			{
				if(cur_ms-g_led_contorl.start_time>POWER_ON_LONG_TIME)
				{
					LED_R_OFF();
					g_led_contorl.led_ope_time = cur_ms;
					g_led_contorl.sub_status=2;
				}
			}
			else
			{
				if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
				{
					LED_INVERT(LED_R_PIN);
					g_led_contorl.led_ope_time = cur_ms;
				}
			}			
		}
			break;
		case LED_ADV_STATUS:
		{
			cur_ms = rtc_timestamp_get_ms();
			if(cur_ms-g_led_contorl.start_time>g_led_contorl.duration)
			{
				LED_R_OFF();
			}
			else
			{
				if(!g_led_contorl.led_ope_time)
				{
					LED_R_ON();
					g_led_contorl.led_ope_time = cur_ms;
				}
				else
				{
					if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
					{
						LED_INVERT(LED_R_PIN);
						g_led_contorl.led_ope_time = cur_ms;
					}
				}
			}
		}
			break;
		case LED_CONNECT_STATUS:
			LED_R_ON();
			break;
		case LED_SEND_DATA_STATUS:
		{
			cur_ms = rtc_timestamp_get_ms();
			{
				if(!g_led_contorl.led_ope_time)
				{
					LED_R_ON();
					g_led_contorl.led_ope_time = cur_ms;
				}
				else
				{
					if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
					{
						LED_INVERT(LED_R_PIN);
						g_led_contorl.led_ope_time = cur_ms;
					}
				}
			}
		}
			break;
		case LED_BATT_WARNING:
		{
			cur_ms = rtc_timestamp_get_ms();
			{
				if(!g_led_contorl.led_ope_time)
				{
					LED_R_ON();
					LED_G_OFF();
					g_led_contorl.led_ope_time = cur_ms;
				}
				else
				{
					if(cur_ms-g_led_contorl.led_ope_time>g_led_contorl.interval_time)
					{
						LED_INVERT(LED_R_PIN);
						g_led_contorl.led_ope_time = cur_ms;
					}
				}
			}
		}
			break;
		default:
			break;
	}
}

void ctl_led_status_set(led_status_t led_status)
{
	switch (led_status)
	{
		case LED_POWER_ON_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 1000;
			break;
		case LED_ADV_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.duration		= 5*1000;
			g_led_contorl.interval_time	= 500;
			break;
		case LED_CONNECT_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 800;
			break;
		case LED_SEND_DATA_STATUS:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 200;
			break;
		case LED_BATT_WARNING:
			memset(&g_led_contorl,0,sizeof(led_contorl_t));
			g_led_contorl.led_status	= led_status;
			g_led_contorl.start_time	= rtc_timestamp_get_ms();
			g_led_contorl.interval_time	= 100;
			break;
		default:
			break;
	}
}

void ctl_led_init(void)
{
	LED_INIT();
	LED_R_ON();
	LED_G_ON();
}
#endif

