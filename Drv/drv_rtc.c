
#include "nrf_error.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_rtc.h"
#include "nrf_rtc.h"
#include "ctl_common.h"

//#include "flex_common.h"
#include "drv_rtc.h"

#define RTC1_MAX_TICKS                0x1000000
#define RTC1_MAX_MSECS                (RTC1_MAX_TICKS / RTC_DEFAULT_CONFIG_FREQUENCY * 1000)
#define RTC1_MIN_MSECS                (1000 / RTC_DEFAULT_CONFIG_FREQUENCY)

static nrf_drv_state_t m_drv_state = NRF_DRV_STATE_UNINITIALIZED;
static uint32_t rtc1_overflow = 0;

typedef struct {
	bool                        in_use;
	flex_rtc_timer_handler_t    handler;
} rtc1_timer_t;

static rtc1_timer_t rtc1_timers[] = {
	[RTC_CC_CHANNEL_0] = {
		.in_use		= false,
		.handler		= NULL
	},

	[RTC_CC_CHANNEL_1] = {
		.in_use		= false,
		.handler		= NULL
	},

	[RTC_CC_CHANNEL_2] = {
		.in_use		= false,
		.handler		= NULL
	},

	[RTC_CC_CHANNEL_3] = {
		.in_use		= false,
		.handler		= NULL
	}
};

static flex_rtc_timer_handler_t overflow_handler = NULL;

static const nrf_drv_rtc_t rtc1 = NRF_DRV_RTC_INSTANCE(2);
static uint8_t free_cc_channels = RTC_CC_CHANNEL_MAX;

static ret_code_t rtc_free_cc_channel_id_get(rtc_cc_channel_id_t * p_channel_id)
{
	VERIFY_PARAM_NOT_NULL(p_channel_id);

	if(free_cc_channels == 0) {
		//flex_error("No free rtc timer channel\r\n");
		return NRF_ERROR_NO_MEM;
	}

	for(rtc_cc_channel_id_t id=RTC_CC_CHANNEL_0; id<RTC_CC_CHANNEL_MAX; id++) {
		if(!rtc1_timers[id].in_use) {
			*p_channel_id = id;
			break;
		}
	}

	return NRF_SUCCESS;
}

static void flex_rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
	rtc_cc_channel_id_t ch = (rtc_cc_channel_id_t)int_type;

	if(ch < RTC_CC_CHANNEL_MAX) {
		free_cc_channels++;
		if(rtc1_timers[ch].handler)
			rtc1_timers[ch].handler();

		rtc1_timers[ch].handler = NULL;
		rtc1_timers[ch].in_use = false;
		
	} else if(int_type == NRF_DRV_RTC_INT_OVERFLOW) {		
		rtc1_overflow++;
		if(overflow_handler != NULL)
			overflow_handler();
	}
}

ret_code_t rtc_timer_stop(flex_rtc_timer_t * p_timer)
{
	VERIFY_PARAM_NOT_NULL(p_timer);

	if(m_drv_state != NRF_DRV_STATE_INITIALIZED)
		return NRF_ERROR_INVALID_STATE;

	if(p_timer->channel_id >= RTC_CC_CHANNEL_MAX)
		return NRF_ERROR_INVALID_PARAM;

	if(rtc1_timers[p_timer->channel_id].in_use) {
		nrf_drv_rtc_cc_disable(&rtc1, p_timer->channel_id);
		rtc1_timers[p_timer->channel_id].handler = NULL;
		rtc1_timers[p_timer->channel_id].in_use = false;
		free_cc_channels++;

	}

	return NRF_SUCCESS;
}

ret_code_t rtc_timer_start(flex_rtc_timer_t * p_timer)
{
	rtc_cc_channel_id_t channel_id;
	uint32_t            ticks;
	ret_code_t          err_code;

	VERIFY_PARAM_NOT_NULL(p_timer);

	if(m_drv_state != NRF_DRV_STATE_INITIALIZED)
		return NRF_ERROR_INVALID_STATE;

	ticks = nrf_drv_rtc_counter_get(&rtc1);

	if(p_timer->msec <= RTC1_MIN_MSECS || p_timer->msec >= RTC1_MAX_MSECS)
		return NRF_ERROR_DATA_SIZE;

	if(!p_timer->handler)
		return NRF_ERROR_NULL;

	err_code = rtc_free_cc_channel_id_get(&channel_id);
	VERIFY_SUCCESS(err_code);

	free_cc_channels--;
	rtc1_timers[channel_id].in_use = true;
	rtc1_timers[channel_id].handler = p_timer->handler;

	ticks += p_timer->msec * RTC_DEFAULT_CONFIG_FREQUENCY / 1000;
	err_code = nrf_drv_rtc_cc_set(&rtc1, channel_id, ticks, true);
	VERIFY_SUCCESS(err_code);

	p_timer->channel_id = channel_id;

	return NRF_SUCCESS;
}

ret_code_t rtc_overflow_handler_register(flex_rtc_timer_t * p_timer)
{
	VERIFY_PARAM_NOT_NULL(p_timer);

	overflow_handler = p_timer->handler;

	return NRF_SUCCESS;
}

time_t rtc_timestamp_get_ms(void)
{
	uint32_t of, ticks;
	time_t   ms;
	//double	ms_f; 

retry:
	of = rtc1_overflow;
	ticks = nrf_drv_rtc_counter_get(&rtc1);
	
	if(of != rtc1_overflow)
		goto retry;

	ms = (RTC1_MAX_TICKS / RTC_DEFAULT_CONFIG_FREQUENCY) * of*1000;
	ms += (ticks / RTC_DEFAULT_CONFIG_FREQUENCY)*1000;
	ms += ((ticks%RTC_DEFAULT_CONFIG_FREQUENCY)*1000)/RTC_DEFAULT_CONFIG_FREQUENCY;
	//ms_f = (ticks/ RTC_DEFAULT_CONFIG_FREQUENCY)*1000;
	//flex_debug("\r\n ms_f:%f",ms_f);
	return ms;
}

time_t rtc_timestamp_get(void)
{
	uint32_t of, ticks;
	time_t   secs;

retry:
	of = rtc1_overflow;
	ticks = nrf_drv_rtc_counter_get(&rtc1);

	if(of != rtc1_overflow)
		goto retry;

	secs = RTC1_MAX_TICKS / RTC_DEFAULT_CONFIG_FREQUENCY * of;
	secs += ticks / RTC_DEFAULT_CONFIG_FREQUENCY;

	return secs;
}

static ret_code_t lfclk_init(void)
{
	ret_code_t err_code;

	if(!nrf_clock_lf_is_running()) {
		err_code = nrf_drv_clock_init();
		VERIFY_SUCCESS(err_code);
	}
	nrf_drv_clock_lfclk_request(NULL);

	return NRF_SUCCESS;
}

ret_code_t drv_rtc_init(void)
{
	ret_code_t err_code;

	err_code = lfclk_init();
	if(err_code != NRF_SUCCESS)
		return err_code;

	nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
	err_code = nrf_drv_rtc_init(&rtc1, &config, flex_rtc_handler);
	VERIFY_SUCCESS(err_code);

	nrf_drv_rtc_overflow_enable(&rtc1, true);

	nrf_drv_rtc_enable(&rtc1);

	m_drv_state = NRF_DRV_STATE_INITIALIZED;

	return NRF_SUCCESS;
}

