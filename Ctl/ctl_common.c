
#include "ctl_common.h"


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	flex_debug("%s: error id %#x, pc addr %#x\r\n", __func__, id, pc);
	nrf_delay_ms(1000);
	NVIC_SystemReset();
}

void print_hex(const data_t * p_data, length_t len)
{
	VERIFY_PARAM_NOT_NULL_VOID(p_data);

	for(int i=0; i<len; i++) {
		if(i%16 == 0)
			flex_debug("\r\n%-*", 16, "");
		flex_debug("%02x ", p_data[i]);
        nrf_delay_ms(1);
	}

	flex_debug("\r\n");
}

uint32_t random_number_get(void)
{
	uint8_t  buf[4];
	uint8_t  size;

	do {
		sd_rand_application_bytes_available_get(&size);
	} while(size < 4);

	sd_rand_application_vector_get(buf, 4);

	return uint32_decode(buf);
}
uint16_t check_sum(uint8_t *data, uint16_t len)
{
	uint16_t sum = 0;
	//sum = len;
	for(uint8_t i=0; i<len; i++)
	{
		sum += data[i];
	}
	return sum;	
}
