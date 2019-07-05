#ifndef __CTL_BATT_H__
#define __CTL_BATT_H__

#include <stdint.h>
#include <stdbool.h>

#define BATTERY_LOW			2000

void ctl_batt_init(void);

uint16_t ctl_batt_get(void);

bool battery_sampling_is_finished(void);

void ctl_batt_check(void);

void stop_batt_timer(void);

void start_batt_timer(void);


#endif
