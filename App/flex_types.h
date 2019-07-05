
#ifndef __FLEX_TYPES_H__
#define __FLEX_TYPES_H__

#include <stdint.h>
#include <stdbool.h>

#include "nrf.h"

typedef uint32_t  ret_code_t;

//flex battery
typedef uint16_t  mv_t;

//flex date
typedef uint32_t  time_t;
typedef uint8_t   day_t;

//flex flash
typedef uint16_t  flash_file_id_t;
typedef uint16_t  flash_file_key_t;
typedef uint8_t * flash_addr_t;

//flex knob
typedef int16_t   degree_t;
typedef uint16_t  pos_t;
typedef int16_t   pos_offset_t;

//flex log
typedef uint16_t  flex_key_id_t;
typedef uint16_t  flex_key_size_t;

//flex magnet
typedef uint16_t  angle_t;
typedef int16_t   angle_offset_t;

//flex ble
typedef uint8_t   data_t;
typedef uint16_t  length_t;

typedef enum {
	FLEX_SUCCESS,
	FLEX_ERROR_MASK                   = 0x40,
	FLEX_ERROR_OUT_OF_MEMORY,
	FLEX_ERROR_NOT_FOUND_LED,
	FLEX_ERROR_NOT_FOUND_FPC,
	FLEX_ERROR_NOT_FOUND_MAGNET,
	FLEX_ERROR_NO_FREE_TIMER_CHANNEL,
	FLEX_ERROR_STOP_MOTOR_TIMEOUT,
	FLEX_ERROR_ROTATE_TIMEOUT,
	FLEX_ERROR_I2C_WRITE,
	FLEX_ERROR_NOT_SUPPORT,
} flex_error_code_t;

typedef enum {
	DIR_CW,  //clockwise; lock dir
	DIR_CCW, //counter clockwise; unlock dir
} dir_t;

typedef enum {
	STATE_UNKNOWN,
	STATE_LOCKED,
	STATE_UNLOCKED,
	HAND_LOCKED,
	HAND_UNLOCKED,
	APP_LOCKED,
	APP_UNLOCKED,
	PWD_LOCKED,
	PWD_UNLOCKED,
	TIMER_LOCKED,
	TIMER_UNLOCKED,
	SELF_LOCKED,
} lock_state_t;

#endif
