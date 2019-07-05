#ifndef _CTL_ICM_FLASH_H_
#define	_CTL_ICM_FLASH_H_

#include <stdint.h>
#include "drv_icm20602.h"
void ctl_flash_icm_data_write(uint8_t *buf, uint32_t len);
void ctl_flash_icm_data_read(uint32_t count);
void ctl_flash_icm_data_read_from_0(uint32_t count);
void ctl_flash_icm_data_read_last(icm_data_t *icm_data);
void ctl_flash_icm_data_init(void);
void ctl_flash_icm_storage(void);
void ctl_icm_data_read_set(uint32_t count);
uint32_t ctl_flash_icm_got_last_index(void);

#endif
