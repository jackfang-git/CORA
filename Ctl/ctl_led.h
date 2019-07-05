#ifndef __CTL_LED_H__
#define __CTL_LED_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum{
	LED_OFF_STATUS,
	LED_POWER_ON_STATUS,
	LED_ADV_STATUS,
	LED_CONNECT_STATUS,
	LED_SEND_DATA_STATUS,
	LED_BATT_WARNING,
}led_status_t;

typedef __packed struct {
	led_status_t led_status;
	uint8_t 	sub_status;
	uint32_t	start_time;
	uint32_t	duration;
	uint16_t	interval_time;
	bool		led_onoff;
	uint32_t	led_ope_time;
}led_contorl_t;

void ctl_led_init(void);
void ctl_led_status_set(led_status_t led_status);
void ctl_led_status_handle(void);




#endif

