#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__

#include "flex_types.h"

typedef enum {
	RTC_CC_CHANNEL_0,
	RTC_CC_CHANNEL_1,
	RTC_CC_CHANNEL_2,
	RTC_CC_CHANNEL_3,
	RTC_CC_CHANNEL_MAX,
} rtc_cc_channel_id_t;

typedef void (* flex_rtc_timer_handler_t) (void);

typedef struct {
	rtc_cc_channel_id_t         channel_id;
	uint32_t                    msec;
	flex_rtc_timer_handler_t    handler;
} flex_rtc_timer_t;

ret_code_t drv_rtc_init(void);
ret_code_t rtc_timer_start(flex_rtc_timer_t *);
ret_code_t rtc_timer_stop(flex_rtc_timer_t *);
ret_code_t rtc_overflow_handler_register(flex_rtc_timer_t *);
time_t rtc_timestamp_get(void);
time_t rtc_timestamp_get_ms(void);

#endif
