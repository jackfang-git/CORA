
#include "fstorage.h"
#include "fds.h"
#include "fds_internal_defs.h"
#include "softdevice_handler.h"

#include "ctl_common.h"
#include "drv_fds_flash.h"

#define FDS_RECORD_ID_INVALID            0xFFFFFFFF

const fds_evt_id_t FDS_EVT_ID_INVALID  = (fds_evt_id_t)0xFF;

typedef struct {
	ret_code_t      result;
	fds_evt_id_t    evt_id;
	uint32_t        rec_id;
	bool            initialed;
} flash_status_t;

static flash_status_t m_flash_status;

static ret_code_t wait_for_fds_event(fds_evt_id_t evt_id, uint32_t record_id)
{
	ret_code_t result = FDS_ERR_BUSY;
	uint16_t   retry = 500;

	while(true) {
		if(m_flash_status.evt_id != evt_id || m_flash_status.rec_id != record_id) {
			retry--;
		} else {
			retry = 0;
			result = m_flash_status.result;
		}

		if(retry)
			nrf_delay_ms(1);
		else
			break;
	}

	return result;
}

static void flex_flash_gc(void)
{
	fds_stat_t state;

	fds_stat(&state);

/*	flex_debug("words_used : %d\r\n", state.words_used);
	flex_debug("open_records : %d\r\n", state.open_records);
	flex_debug("valid_record : %d\r\n", state.valid_records);
	flex_debug("largest_contig : %d\r\n", state.largest_contig);
	flex_debug("freeable_words : %d\r\n", state.freeable_words); */
	if(state.freeable_words > FDS_PAGE_SIZE) {
		fds_gc();
		wait_for_fds_event(FDS_EVT_GC, FDS_RECORD_ID_INVALID);
	}
}

static void fds_event_handler(fds_evt_t const * const p_evt)
{
	switch(p_evt->id) {
		case FDS_EVT_INIT:
			m_flash_status.result = p_evt->result;
			m_flash_status.evt_id = p_evt->id;
			m_flash_status.rec_id = FDS_RECORD_ID_INVALID;
			m_flash_status.initialed = true;
			break;

		case FDS_EVT_WRITE:
		case FDS_EVT_UPDATE:
			m_flash_status.result = p_evt->result;
			m_flash_status.evt_id = p_evt->id;
			m_flash_status.rec_id = p_evt->write.record_id;
			break;

		case FDS_EVT_DEL_RECORD:
		case FDS_EVT_DEL_FILE:
		case FDS_EVT_GC:
			m_flash_status.result = p_evt->result;
			m_flash_status.evt_id = p_evt->id;
			m_flash_status.rec_id = FDS_RECORD_ID_INVALID;
			break;

		default:
			break;
	}
}

ret_code_t flex_flash_erase_file(flash_file_id_t id)
{
	if(!m_flash_status.initialed)
		return FDS_ERR_NOT_INITIALIZED;

	ret_code_t err_code = fds_file_delete(id);
	VERIFY_SUCCESS(err_code);

	wait_for_fds_event(FDS_EVT_DEL_FILE, FDS_RECORD_ID_INVALID);

	flex_flash_gc();

	return NRF_SUCCESS;
}

ret_code_t flex_flash_erase_record(flash_file_id_t id, flash_file_key_t key)
{
	fds_record_desc_t  desc;
	ret_code_t err_code=0;
	fds_find_token_t   token = {0};

	if(!m_flash_status.initialed)
		return FDS_ERR_NOT_INITIALIZED;

	err_code = fds_record_find(id, key, &desc, &token);
	VERIFY_SUCCESS(err_code);

	err_code = fds_record_delete(&desc);
	VERIFY_SUCCESS(err_code);

	wait_for_fds_event(FDS_EVT_DEL_RECORD, FDS_RECORD_ID_INVALID);
	flex_flash_gc();
/*
	fds_gc();
	wait_for_fds_event(FDS_EVT_GC, FDS_RECORD_ID_INVALID);
*/
	return NRF_SUCCESS;
}

ret_code_t flex_flash_read_record(flash_file_id_t     id,
                                  flash_file_key_t    key,
                                  flex_flash_data_t * p_flash_data)
{
	fds_record_desc_t  desc;
	fds_flash_record_t flash;
	fds_find_token_t   token = {0};
	uint16_t           wlen;
	uint32_t         * p_data;

	VERIFY_PARAM_NOT_NULL(p_flash_data);

	if(!m_flash_status.initialed)
		return FDS_ERR_NOT_INITIALIZED;

	ret_code_t err_code = fds_record_find(id, key, &desc, &token);
	VERIFY_SUCCESS(err_code);

	err_code = fds_record_open(&desc, &flash);
	VERIFY_SUCCESS(err_code);

	wlen = flash.p_header->tl.length_words;
	p_data = (uint32_t *)p_flash_data->addr;
	for(uint16_t i=0; i<wlen; i++)
		p_data[i] = *((uint32_t *)(flash.p_data) + i);

	p_flash_data->length = wlen << 2;

	fds_record_close(&desc);

	return NRF_SUCCESS;
}

ret_code_t flex_flash_write_record(flash_file_id_t                 id,
                                   flash_file_key_t                key,
                                   flex_flash_data_t const * const p_flash_data)
{
	fds_record_t       record;
	fds_record_chunk_t chunk;
	fds_record_desc_t  desc;
	fds_find_token_t   token = {0};
	static uint32_t    times = 0;
	data_t             p_data[FLASH_REC_LEN_MAX];

	VERIFY_PARAM_NOT_NULL(p_flash_data);
	VERIFY_PARAM_NOT_NULL(p_flash_data->addr);

	if(!m_flash_status.initialed)
		return FDS_ERR_NOT_INITIALIZED;

	memset(p_data, 0, sizeof(p_data));
	memcpy(p_data, p_flash_data->addr, p_flash_data->length);  //because p_flash_data->addr may not ALIGN 4

	chunk.p_data           = p_data;
	chunk.length_words     = BYTES_TO_WORDS(MIN(p_flash_data->length, FLASH_REC_LEN_MAX));

	record.file_id         = id;
	record.key             = key;
	record.data.p_chunks   = &chunk;
	record.data.num_chunks = 1;

	ret_code_t err_code = fds_record_find(id, key, &desc, &token);
	//flex_debug("\r\n find error:%d",err_code);
	if(err_code == FDS_ERR_NOT_FOUND) {
		err_code = fds_record_write(&desc, &record);
		VERIFY_SUCCESS(err_code);
		wait_for_fds_event(FDS_EVT_WRITE, desc.record_id);
	} else {
		err_code = fds_record_update(&desc, &record);
		VERIFY_SUCCESS(err_code);
		wait_for_fds_event(FDS_EVT_UPDATE, desc.record_id);
	}

	if(++times > 1000) {
		flex_flash_gc();
		times = 0;
	}
	return NRF_SUCCESS;
}

ret_code_t drv_fds_flash_init(void)
{
	ret_code_t err_code;

	m_flash_status.result = FDS_SUCCESS;
	m_flash_status.evt_id = FDS_EVT_ID_INVALID;
	m_flash_status.rec_id = FDS_RECORD_ID_INVALID;
	m_flash_status.initialed = false;

#ifndef SOFTDEVICE_PRESENT
	err_code = softdevice_sys_evt_handler_set(fs_sys_event_handler);
	if(err_code != NRF_SUCCESS)
		return err_code;
#endif

	err_code = fds_register(fds_event_handler);
	VERIFY_SUCCESS(err_code);
	err_code = fds_init();
    
	VERIFY_SUCCESS(err_code);

	wait_for_fds_event(FDS_EVT_INIT, FDS_RECORD_ID_INVALID);

	return NRF_SUCCESS;
}
