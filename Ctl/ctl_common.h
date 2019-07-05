
#ifndef __CTL_COMMON_H__
#define __CTL_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "app_error.h"
#include "app_util.h"

#include "sdk_common.h"

#include "nordic_common.h"
#include "nrf_soc.h"
#include "nrf_delay.h"
#include "nrf_drv_common.h"

#include "flex_types.h"
#include "flex_debug.h"

#define VERSION		0x00000006


void print_hex(const data_t *, length_t);
void print_hex_err(const data_t *, length_t);
uint32_t random_number_get(void);
uint16_t check_sum(uint8_t *data, uint16_t len);

#endif
