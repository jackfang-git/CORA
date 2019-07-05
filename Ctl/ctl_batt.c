#include "nrf_delay.h"
#include "nrf_drv_saadc.h"
#include "ctl_batt.h"
#include "ctl_common.h"
#include "ctl_led.h"

#include "ctl_cmd.h"
#include "ctl_cmd_queue.h"
#include "drv_rtc.h"


#define SAMPLES_IN_BUFFER   5
#define BATT_INPUT_AIN		NRF_SAADC_INPUT_AIN1

#define BATT_FACTOR				829		  // actual factor is BATT_FACTOR/FULL_SAMPLE_VALUE 829/100=8.29 but its not support float
#define FULL_SAMPLE_VALUE   	100


static uint32_t g_battery_quantity=0;
static bool sampling_finish_flag = true;
static int16_t g_buffer_pool[SAMPLES_IN_BUFFER];

static void batt_samp_wait_for_result(void)
{
    uint16_t retry=100;//100
    while(true)
    {
        if(sampling_finish_flag==true)
        {
            break;
        }
        else
        {
            nrf_delay_ms(1);
            retry--;
            if(retry==0)
            {
                break;
            }
        }
    }
}

bool battery_sampling_is_finished(void)
{
	return sampling_finish_flag;
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        sampling_finish_flag = true;
    }
}

static void ADC_value_convert_to_vol(void)
{
    int16_t sampling_avg=0;
    uint16_t sampling_avg_u=0;
    for(uint8_t i=0;i<SAMPLES_IN_BUFFER;i++)
    {
        sampling_avg+=g_buffer_pool[i];
    }
    if(sampling_avg<0)
        sampling_avg_u = 0;
    else
        sampling_avg_u = sampling_avg;
    sampling_avg_u = sampling_avg_u/SAMPLES_IN_BUFFER;
	flex_debug("\r\n vol:%d",sampling_avg_u);
	print_hex((uint8_t *)&g_buffer_pool[0],10);
	g_battery_quantity = (sampling_avg_u*BATT_FACTOR)/FULL_SAMPLE_VALUE;
}


uint16_t ctl_batt_get(void)
{
	ret_code_t err_code=0;
	
	sampling_finish_flag = false;
	memset(g_buffer_pool,0,5);
    err_code = nrf_drv_saadc_buffer_convert(g_buffer_pool, SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
    for(uint8_t index=0;index<SAMPLES_IN_BUFFER;index++)
    {
        nrf_delay_ms(1);
        nrf_drv_saadc_sample();
    }
    batt_samp_wait_for_result();
    ADC_value_convert_to_vol();
	
    return g_battery_quantity;
	
}


void ctl_batt_check(void)
{
	while(!battery_sampling_is_finished());
	if(ctl_batt_get()<BATTERY_LOW)
		ctl_led_status_set(LED_BATT_WARNING);
	else
	{
		while(!battery_sampling_is_finished());
		if(ctl_batt_get()<BATTERY_LOW)
			ctl_led_status_set(LED_BATT_WARNING);
	}
}

void ctl_batt_init(void)
{
	ret_code_t err_code;
    
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(BATT_INPUT_AIN); //NRF_SAADC_INPUT_AIN1

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

}

#define TIMER_BATT_TIMEOUT		4000

static uint8_t timer_batt_flag = 0;
static void timer_batt_handler(void);
static flex_rtc_timer_t timer_batt={RTC_CC_CHANNEL_0,TIMER_BATT_TIMEOUT,timer_batt_handler};


void stop_batt_timer(void)
{
	if(timer_batt_flag==1)
		rtc_timer_stop(&timer_batt);
	timer_batt_flag = 0;
}

void start_batt_timer(void)
{
	if(timer_batt_flag==0)
		rtc_timer_start(&timer_batt);
	else
	{
		stop_batt_timer();
	}
	timer_batt_flag = 1;
}

static void timer_batt_handler(void)
{
	cmd_t cmd;

	timer_batt_flag = 0;
	
	memset(&cmd, 0, sizeof(cmd));
	cmd.cid   		= CMD_READ_BATT;
	cmd.param_len 	= 0;	
	cmd.cmd_type    = CMD_PERM_SYS;
	ctl_cmd_enqueue(&cmd);
	
	start_batt_timer();
}









