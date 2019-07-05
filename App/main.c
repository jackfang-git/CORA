/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */
#include "ctl_common.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_util_platform.h"
#include "drv_rtc.h"
#include "drv_icm20602.h"
#include "ctl_ble.h"
#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#include "ctl_icm20602_queue.h"
#include "ctl_icm20602_timer.h"
#include "ctl_cmd.h"
#include "nrf_gpio.h"
#include "drv_xt25q32.h"
#include "ctl_icm_flash.h"
#include "drv_fds_flash.h"
#include "ctl_led.h"
#include "ctl_batt.h"



/**@brief Function for placing the application in low power state while waiting for events.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Application main function.
 */

int main(void)
{
	flex_debug("\r\n start...\r\n");
	drv_rtc_init();

	drv_icm_init();

	drv_xt25q_init();
	
	ctl_led_init();
	
	ctl_batt_init();
	
	ctl_cmd_init();

	ctl_ble_init();  

	ctl_icm20602_timer_init();	
	
	ctl_led_status_set(LED_POWER_ON_STATUS);
	
	//ctl_batt_check();
	
	flex_debug("\r\n start...%d\r\n",rtc_timestamp_get());
	
    // Enter main loop.
    for (;;)
    {
		if(!ctl_cmd_queue_is_empty())
		{
			ctl_cmd_parse();
		}
		while(queue_data_rebuf_size()!=0)
		{
			ctl_flash_icm_storage();
		}
		ctl_led_status_handle();
        power_manage();
    }
}


/**
 * @}
 */
