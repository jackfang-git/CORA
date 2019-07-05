#ifndef DRV_ICM20602_H_
#define DRV_ICM20602_H_

#include <stdbool.h>
#include <stdint.h>
#include "flex_types.h"

#define DISABLE_ICM20602       1
#define ENABLE_ICM20602        0

#define GYRO 					1
#define ICM_ACC  					0

#define PWR_MGMT_1				0X6B
#define SIGNAL_PATH_RESET		0X68
#define SMPLRT_DIV				0X19
#define ICM_CONFIG				0X1A
#define ACCEL_CONFIG2			0X1D
#define INT_ENABLE				0X38
#define FIFO_EN					0X23
#define INT_PIN_CFG				0X37

#define ACCEL_XOUT_H			0X3B
#define ACCEL_XOUT_L			0X3C
#define ACCEL_YOUT_H			0X3D
#define ACCEL_YOUT_L			0X3E
#define ACCEL_ZOUT_H			0X3F
#define ACCEL_ZOUT_L			0X40

#define TEMP_OUT_H				0X41
#define TEMP_OUT_L				0X42

#define GYRO_XOUT_H				0X43
#define GYRO_XOUT_L				0X44
#define GYRO_YOUT_H				0X45
#define GYRO_YOUT_L				0X46
#define GYRO_ZOUT_H				0X47
#define GYRO_ZOUT_L				0X48

#define TEMP_OUT_H				0X41
#define TEMP_OUT_L				0X42

#define SELF_TEST_X_ACCEL		0X0D	
#define SELF_TEST_Y_ACCEL		0X0E	
#define SELF_TEST_Z_ACCEL		0X0F	
#define ACCEL_CONFIG			0X1C	

#define GYRO_CONFIG				0X1B	
#define SELF_TEST_X_GYRO		0X50	
#define SELF_TEST_Y_GYRO		0X51	
#define SELF_TEST_Z_GYRO		0X52	

#define WHO_AM_I         		0X75	
#define ICM20602_WHO_AM_I_CONST		(0X12)


typedef __packed struct {
	short gx;
	short gy;
	short gz;
	short ax;
	short ay;
	short az;
} icm20602_data_t;

typedef __packed struct{
	time_t	ms;
	icm20602_data_t	icm20602_data;
}icm_data_t;

uint8_t drv_icm_init(void);
uint8_t icm20602SpiDetect(void);
int16_t ICM_Get_Temperature(void);
uint8_t ICM_Get_Gyroscope(short *gx,short *gy,short *gz);
uint8_t ICM_Get_Accelerometer(short *ax,short *ay,short *az);

uint8_t Temp_SelfTest(void);
uint8_t Gyro_SelfTest(void);
uint8_t Acc_SelfTest(void);

void drv_icm_set_gyro_fsr(uint8_t fsr);
void drv_icm_set_acc_fsr(uint8_t fsr);
void drv_icm_set_gyro_lpf(uint16_t lpf);
void drv_icm_set_acc_lpf(uint16_t lpf);

void drv_icm_switch(bool enable);

void ICM_Print(void);

#endif
