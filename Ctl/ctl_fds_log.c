
#include "ctl_common.h"
#include "drv_fds_flash.h"
#include "ctl_fds_log.h"
#include "drv_rtc.h"

#define FLEX_LOG_HEAD_ID                0xBFFE  //this is key id, every log file have a log head

static const flex_key_id_t key_end_id[] = {
	[FLEX_LOG_ICM_DATA_HEAD_ID]       = 0x01,
};

static bool flex_log_check_id(flex_log_id_t log_id, flex_key_id_t key_id)
{
	if(log_id >= FLEX_LOG_ID_END)
		return false;

	return key_id <= key_end_id[log_id];
}

static ret_code_t flex_log_head_update(flex_log_id_t log_id, flex_log_head_t * p_log_head)
{
	flex_flash_data_t flash_data;

	VERIFY_PARAM_NOT_NULL(p_log_head);

	flash_data.addr   = (flash_addr_t)p_log_head;
	flash_data.length = sizeof(flex_log_head_t);

	return flex_flash_write_record(log_id, FLEX_LOG_HEAD_ID, &flash_data);
}

ret_code_t flex_log_head_read(flex_log_id_t log_id, flex_log_head_t * p_log_head)
{
	flex_flash_data_t flash_data;
	ret_code_t        err_code;
	data_t            p_data[FLASH_REC_LEN_MAX];

	VERIFY_PARAM_NOT_NULL(p_log_head);

	flash_data.addr   = (flash_addr_t)p_data;
	flash_data.length = sizeof(p_data);
	memset(flash_data.addr, 0, flash_data.length);

	err_code = flex_flash_read_record(log_id, FLEX_LOG_HEAD_ID, &flash_data);
	VERIFY_SUCCESS(err_code);

	memcpy(p_log_head, p_data, sizeof(flex_log_head_t));

	return NRF_SUCCESS;
}

ret_code_t flex_log_read_hex(flex_log_id_t log_id, flex_key_id_t key_id, uint8_array_t * p_flex_log)
{
	flex_flash_data_t flash_data;
	flex_log_head_t   log_head;
	flex_key_id_t     last_id;
	ret_code_t        err_code;
	data_t            p_data[FLASH_REC_LEN_MAX];

	VERIFY_PARAM_NOT_NULL(p_flex_log);
	VERIFY_PARAM_NOT_NULL(p_flex_log->p_data);

	//flex_debug("\r\n%s log_id %d key_id %d ", __func__, log_id, key_id);

	if(flex_log_check_id(log_id, key_id) == false)
		return NRF_ERROR_INVALID_PARAM;

	err_code = flex_log_head_read(log_id, &log_head);
	if(err_code != NRF_SUCCESS) {
		log_head.start_id = 1;
		log_head.last_id  = 0;
		log_head.end_id   = key_end_id[log_id];
		log_head.size     = 0;
	}

	//flex_debug("\r\nstart_id %d last_id %d size %d\r\n", log_head.start_id, log_head.last_id, log_head.size);

	if(key_id == 0)
		last_id = log_head.last_id;
	else
		last_id = key_id;

	if(flex_log_check_id(log_id, last_id) == false)
		return NRF_ERROR_INVALID_PARAM;

	flash_data.addr   = (flash_addr_t)p_data;
	flash_data.length = sizeof(p_data);
	memset(flash_data.addr, 0, flash_data.length);

	err_code = flex_flash_read_record(log_id, last_id, &flash_data);
	VERIFY_SUCCESS(err_code);

	p_flex_log->size = MIN(p_flex_log->size, flash_data.length);  //be careful
	memcpy(p_flex_log->p_data, p_data, p_flex_log->size);

	//print_hex(p_flex_log->p_data, p_flex_log->size);

	return NRF_SUCCESS;
}

ret_code_t flex_log_save_hex(flex_log_id_t log_id, flex_key_id_t key_id, uint8_array_t * p_flex_log)
{
	flex_flash_data_t flash_data;
	flex_log_head_t   log_head;
	flex_key_id_t     last_id;
	ret_code_t        err_code;

	VERIFY_PARAM_NOT_NULL(p_flex_log);
	VERIFY_PARAM_NOT_NULL(p_flex_log->p_data);

	//flex_debug("\r\n%s log_id %d key_id %d ", __func__, log_id, key_id);

	if(p_flex_log->size > FLASH_REC_LEN_MAX)
		return NRF_ERROR_DATA_SIZE;

	err_code = flex_log_head_read(log_id, &log_head);
	//flex_debug("\r\n flex_log_head_read\r\n");
	if(err_code != NRF_SUCCESS) {
		log_head.start_id  = 1;
		log_head.last_id   = 0;
		log_head.end_id    = key_end_id[log_id];
		log_head.size      = 0;
		log_head.overwrite = 0;
	}
	log_head.end_id    = key_end_id[log_id];

	if(key_id == 0) {
		last_id = log_head.last_id + 1;
		if(last_id > log_head.end_id)
			last_id = 1;
	} else {
		last_id = key_id;
	}

	if(flex_log_check_id(log_id, last_id) == false)
        flex_debug("\r\nNRF_ERROR_INVALID_PARAM\r\n");
		//return NRF_ERROR_INVALID_PARAM;

	flash_data.addr   = (flash_addr_t)p_flex_log->p_data;
	flash_data.length = p_flex_log->size;

	err_code = flex_flash_write_record(log_id, last_id, &flash_data);
	VERIFY_SUCCESS(err_code);

	//update log head
	log_head.last_id = last_id;
	log_head.size    = MIN(log_head.size+1, log_head.end_id);

	if(log_head.size == log_head.end_id && log_head.start_id == last_id) {
		if(++(log_head.start_id) > log_head.end_id)
			log_head.start_id = 1;

		log_head.overwrite++;
	}

	err_code = flex_log_head_update(log_id, &log_head);

	return err_code;
}

ret_code_t flex_log_save_hex_key(flex_log_id_t log_id, flex_key_id_t key_id, uint8_array_t * p_flex_log, uint8_t * key)
{
	flex_flash_data_t flash_data;
	flex_log_head_t   log_head;
	flex_key_id_t     last_id;
	ret_code_t        err_code;

	VERIFY_PARAM_NOT_NULL(p_flex_log);
	VERIFY_PARAM_NOT_NULL(p_flex_log->p_data);

	//flex_debug("\r\n%s log_id %d key_id %d ", __func__, log_id, key_id);

	if(p_flex_log->size > FLASH_REC_LEN_MAX)
		return NRF_ERROR_DATA_SIZE;

	err_code = flex_log_head_read(log_id, &log_head);
	//flex_debug("\r\n flex_log_head_read\r\n");
	if(err_code != NRF_SUCCESS) {
		log_head.start_id  = 1;
		log_head.last_id   = 0;
		log_head.end_id    = key_end_id[log_id];
		log_head.size      = 0;
		log_head.overwrite = 0;
	}
	log_head.end_id    = key_end_id[log_id];

	if(key_id == 0) {
		last_id = log_head.last_id + 1;
		if(last_id > log_head.end_id)
			last_id = 1;
	} else {
		last_id = key_id;
	}
	*key = last_id;


	if(flex_log_check_id(log_id, last_id) == false)
        flex_debug("\r\nNRF_ERROR_INVALID_PARAM\r\n");
		//return NRF_ERROR_INVALID_PARAM;

	flash_data.addr   = (flash_addr_t)p_flex_log->p_data;
	flash_data.length = p_flex_log->size;

	err_code = flex_flash_write_record(log_id, last_id, &flash_data);
	VERIFY_SUCCESS(err_code);

	//update log head
	log_head.last_id = last_id;
	log_head.size    = MIN(log_head.size+1, log_head.end_id);

	if(log_head.size == log_head.end_id && log_head.start_id == last_id) {
		if(++(log_head.start_id) > log_head.end_id)
			log_head.start_id = 1;

		log_head.overwrite++;
	}

	err_code = flex_log_head_update(log_id, &log_head);

	return err_code;
}

ret_code_t flex_log_update_hex(flex_log_id_t log_id, flex_key_id_t key_id, uint8_array_t * p_flex_log)
{
	flex_flash_data_t flash_data;
	flex_log_head_t   log_head;
	flex_key_id_t     last_id;
	ret_code_t        err_code;

	VERIFY_PARAM_NOT_NULL(p_flex_log);
	VERIFY_PARAM_NOT_NULL(p_flex_log->p_data);

	//flex_debug("\r\n%s log_id %d key_id %d ", __func__, log_id, key_id);

	if(p_flex_log->size > FLASH_REC_LEN_MAX)
		return NRF_ERROR_DATA_SIZE;

	err_code = flex_log_head_read(log_id, &log_head);
	//flex_debug("\r\n flex_log_head_read\r\n");
	if(err_code != NRF_SUCCESS) {
		log_head.start_id  = 1;
		log_head.last_id   = 0;
		log_head.end_id    = key_end_id[log_id];
		log_head.size      = 0;
		log_head.overwrite = 0;
	}
	log_head.end_id    = key_end_id[log_id];

	if(key_id == 0) {
		last_id = log_head.last_id + 1;
		if(last_id > log_head.end_id)
			last_id = 1;
	} else {
		last_id = key_id;
	}

	if(flex_log_check_id(log_id, last_id) == false)
        flex_debug("\r\nNRF_ERROR_INVALID_PARAM\r\n");
		//return NRF_ERROR_INVALID_PARAM;

	flash_data.addr   = (flash_addr_t)p_flex_log->p_data;
	flash_data.length = p_flex_log->size;

	err_code = flex_flash_write_record(log_id, last_id, &flash_data);
	VERIFY_SUCCESS(err_code);


	return err_code;
}

ret_code_t flex_log_save(flex_log_id_t log_id, char * fmt, ...)
{
	uint8_array_t flex_log;
	time_t        time;
	data_t        p_data[FLASH_REC_LEN_MAX];

	VERIFY_PARAM_NOT_NULL(fmt);

	time = rtc_timestamp_get();

	memset(p_data, 0, sizeof(p_data));
	uint32_encode(time, p_data);  //record time

	va_list ap;
	va_start(ap, fmt);
	vsprintf((char *)p_data+sizeof(time_t), fmt, ap);  //record string
	va_end(ap);

	flex_log.p_data = p_data;
	flex_log.size   = strlen((char *)(p_data + sizeof(time_t))) + sizeof(time_t);

	return flex_log_save_hex(log_id, 0, &flex_log);
}

ret_code_t flex_log_delete_key_id(flex_log_id_t log_id,flex_key_id_t key_id)
{
	flex_log_head_t log_head;
	ret_code_t      err_code;
	
	err_code = flex_flash_erase_record(log_id, key_id);
	VERIFY_SUCCESS(err_code);
	
	while(flex_log_head_read(log_id, &log_head) == NRF_SUCCESS)
		nrf_delay_ms(1);
	
	return NRF_SUCCESS;
}

ret_code_t flex_log_delete_id(flex_log_id_t log_id)
{
	flex_log_head_t log_head;
	ret_code_t      err_code;

	err_code = flex_flash_erase_file(log_id);
	VERIFY_SUCCESS(err_code);

	while(flex_log_head_read(log_id, &log_head) == NRF_SUCCESS)
		nrf_delay_ms(1);

	return NRF_SUCCESS;
}

ret_code_t flex_log_delete(flex_log_id_t log_id, flex_key_size_t log_size)
{
	flex_log_head_t   log_head;
	ret_code_t        err_code;

	err_code = flex_log_head_read(log_id, &log_head);
	VERIFY_SUCCESS(err_code);

	if(log_head.size <= log_size)
		return flex_log_delete_id(log_id);

	log_head.start_id += log_size;
	if(log_head.start_id > log_head.end_id)
		log_head.start_id -= log_head.end_id;

	log_head.size     -= log_size;
	log_head.overwrite = 0;

	return flex_log_head_update(log_id, &log_head);
}

ret_code_t flex_log_delete_all(void)
{
	ret_code_t err_code = NRF_SUCCESS;

	for(flex_log_id_t log_id=FLEX_LOG_ID_START; log_id<FLEX_LOG_ID_END; log_id++) {
		err_code |= flex_log_delete_id(log_id);
	}

	return err_code;
}
