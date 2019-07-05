
#ifndef __CTL_LOG_H__
#define __CTL_LOG_H__

#include "app_util.h"

#include "flex_types.h"
#include "drv_fds_flash.h"

typedef uint16_t flex_key_id_t;
typedef uint16_t flex_key_size_t;

typedef enum {
	FLEX_LOG_ID_START                 = 0x1,
	FLEX_LOG_ICM_DATA_HEAD_ID         = FLEX_LOG_ID_START,
	FLEX_LOG_ID_END = 0xff
} flex_log_id_t;


typedef __packed struct {
	flex_key_id_t      start_id;
	flex_key_id_t      last_id;
	flex_key_id_t      end_id;
	flex_key_size_t    size;
	flex_key_size_t    overwrite;
} flex_log_head_t;

ret_code_t flex_log_head_read(flex_log_id_t, flex_log_head_t *);

ret_code_t flex_log_save(flex_log_id_t, char *, ...);

ret_code_t flex_log_read_hex(flex_log_id_t, flex_key_id_t, uint8_array_t *);
ret_code_t flex_log_save_hex(flex_log_id_t, flex_key_id_t, uint8_array_t *);
ret_code_t flex_log_update_hex(flex_log_id_t, flex_key_id_t, uint8_array_t *);
ret_code_t flex_log_save_hex_key(flex_log_id_t, flex_key_id_t, uint8_array_t *, uint8_t *);
ret_code_t flex_log_delete_id(flex_log_id_t);
ret_code_t flex_log_delete_key_id(flex_log_id_t log_id,flex_key_id_t key_id);
ret_code_t flex_log_delete(flex_log_id_t, flex_key_size_t);
ret_code_t flex_log_delete_all(void);

#endif
