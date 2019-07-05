
#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include "flex_types.h"

#define FLASH_REC_LEN_MAX                32

typedef struct {
	flash_addr_t    addr;
	length_t        length;
} flex_flash_data_t;

ret_code_t drv_fds_flash_init(void);
ret_code_t flex_flash_write_record(flash_file_id_t, flash_file_key_t, flex_flash_data_t const * const);
ret_code_t flex_flash_read_record(flash_file_id_t, flash_file_key_t, flex_flash_data_t *);
ret_code_t flex_flash_erase_record(flash_file_id_t, flash_file_key_t);
ret_code_t flex_flash_erase_file(flash_file_id_t);

#endif
