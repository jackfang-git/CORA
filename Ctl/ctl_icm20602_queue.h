#ifndef _CTL_ICM20602_QUEUE_H_
#define _CTL_ICM20602_QUEUE_H_

#include <stdint.h>
#include "drv_icm20602.h"
#include "drv_rtc.h"

#define QUEUE_BUF_SIZE			80



//101

void queue_data_put(icm_data_t data);
void queue_data_get(icm_data_t *data);
uint8_t queue_data_rebuf_size(void);
void queue_data_get_last(icm_data_t *data);
void queue_data_get_req(uint8_t req_index,icm_data_t *data);
uint8_t quene_get_last_index(void);
uint8_t quene_get_first_index(void);
void ctl_queue_data_init(void);

ret_code_t queue_icm_data_got(void);

#endif
