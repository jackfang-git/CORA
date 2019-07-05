
#ifndef __CTL_CMD_QUEUE_H__
#define __CTL_CMD_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_errors.h"

#define CMD_PARAM_LEN_MAX			19

typedef enum {
	CMD_PERM_SYS,
	CMD_PERM_BLE,
} cmd_type_t;

typedef enum {
	CMD_READ_ICM 		= 0,
	CMD_ICM_DATA_GOT	= 1,
	CMD_ICM_NOTIF		= 2,
	CMD_SET_GYRO_FSR	= 3,
	CMD_SET_ACC_FSR		= 4,
	CMD_SET_GYRO_LPF	= 5,
	CMD_SET_ACC_LPF		= 6,
	CMD_READ_BATT		= 7,
	CMD_RESET			= 0x0e,
	CMD_DFU				= 0x0F,
	CMD_MAX,
}cmd_id_t;
typedef __packed struct {
	cmd_id_t           cid;
	__packed union {
		uint8_t	arg[CMD_PARAM_LEN_MAX];
		uint16_t lpf;
		uint32_t count;
	} param;
	uint8_t param_len;
	cmd_type_t cmd_type;
} cmd_t;

void ctl_cmd_queue_init(void);
ret_code_t ctl_cmd_enqueue(const cmd_t *);
ret_code_t ctl_cmd_dequeue(cmd_t *);
bool ctl_cmd_queue_is_empty(void);

#endif
