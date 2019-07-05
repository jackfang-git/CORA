#include "ctl_common.h"
#include "drv_rtc.h"
#include "ctl_cmd.h"
#include "ctl_cmd_queue.h"
#include "ctl_ble.h"
#include "ctl_icm20602_queue.h"
#include "drv_icm20602.h"
#include "ctl_icm20602_timer.h"
#include "ctl_icm_flash.h"
#include "drv_xt25q32.h"
#include "ctl_led.h"
#include "ctl_batt.h"


typedef ret_code_t (* cmd_handler_t) (cmd_t *);


static ret_code_t cmd_read_icm(cmd_t * p_cmd)
{
	ctl_icm_data_read_set(p_cmd->param.count);
	ctl_led_status_set(LED_SEND_DATA_STATUS);
	return 0;
}

static ret_code_t cmd_icm_data_got(cmd_t * p_cmd)
{
	
	icm_data_t read_buf;
	memset(&read_buf,0,sizeof(icm_data_t));
	ICM_Get_Gyroscope((short *)&read_buf.icm20602_data.gx,(short *)&read_buf.icm20602_data.gy,(short *)&read_buf.icm20602_data.gz);
	ICM_Get_Accelerometer((short *)&read_buf.icm20602_data.ax,(short *)&read_buf.icm20602_data.ay,(short *)&read_buf.icm20602_data.az);
	read_buf.ms = rtc_timestamp_get_ms();
	queue_data_put(read_buf);
	ctl_flash_icm_data_write((uint8_t *)&read_buf,sizeof(icm_data_t));
	return NRF_SUCCESS;
}

static ret_code_t cmd_icm_data_notif(cmd_t * p_cmd)
{
	icm_data_t read_buf;
	
	memset(&read_buf,0,sizeof(icm20602_data_t));	
	ctl_flash_icm_data_read_last(&read_buf);

	//print_hex((uint8_t *)&read_buf,sizeof(icm20602_data_t));
	data_send((uint8_t *)&read_buf,sizeof(icm_data_t));	
	//flex_debug("\r\n ms:%d",read_buf.ms);
	return 0;
}

static ret_code_t cmd_dfu(cmd_t * p_cmd)
{
	cmd_t cmd;

	VERIFY_PARAM_NOT_NULL(p_cmd);

	NRF_POWER->GPREGRET = 0x17;
	memset(&cmd, 0, sizeof(cmd));
	cmd.cid   		= CMD_RESET;
	cmd.param_len 	= 0;	
	cmd.cmd_type    = CMD_PERM_SYS;

	return ctl_cmd_enqueue(&cmd);

}

static ret_code_t cmd_set_gyro_fsr(cmd_t * p_cmd)
{
	uint8_t fsr=0;
	
	fsr = p_cmd->param.arg[0];
	if(fsr>3)
		fsr = 3;
	drv_icm_switch(0);
	drv_icm_set_gyro_fsr(fsr);	
	drv_icm_switch(1);
	return 0;
}
static ret_code_t cmd_set_acc_fsr(cmd_t * p_cmd)
{
	uint8_t fsr=0;
	
	fsr = p_cmd->param.arg[0];
	if(fsr>3)
		fsr = 3;
	drv_icm_switch(0);
	drv_icm_set_acc_fsr(fsr);
	drv_icm_switch(1);
	return 0;
}


static ret_code_t cmd_set_gyro_lpf(cmd_t * p_cmd)
{
	uint16_t lpf=0;
	
	lpf =  p_cmd->param.lpf;
	drv_icm_switch(0);
	drv_icm_set_gyro_lpf(lpf);
	drv_icm_switch(1);
	return 0;
}
static ret_code_t cmd_set_acc_lpf(cmd_t * p_cmd)
{
	uint16_t lpf=0;
	
	lpf =  p_cmd->param.lpf;
	drv_icm_switch(0);
	drv_icm_set_acc_lpf(lpf);
	drv_icm_switch(1);
	return 0;
}

static ret_code_t cmd_reset(cmd_t * p_cmd)
{
	VERIFY_PARAM_NOT_NULL(p_cmd);
	
	NVIC_SystemReset();

	return NRF_SUCCESS;
}

static ret_code_t cmd_read_batt(cmd_t * p_cmd)
{
	VERIFY_PARAM_NOT_NULL(p_cmd);
	
	if(battery_sampling_is_finished())
	{
		if(ctl_batt_get()<BATTERY_LOW)
			ctl_led_status_set(LED_BATT_WARNING);
	}		

	return NRF_SUCCESS;
}

ret_code_t ctl_cmd_parse(void)
{
	cmd_handler_t cmd_execute;
	cmd_t         cmd;
	ret_code_t    err_code;

	memset(&cmd, 0, sizeof(cmd_t));
	err_code = ctl_cmd_dequeue(&cmd);
	VERIFY_SUCCESS(err_code);
/*	flex_debug("cmd");
	print_hex((data_t *)&cmd, cmd.param_len); */

	cmd_execute = NULL;
	err_code = NRF_SUCCESS;

	switch(cmd.cid) {
		case CMD_READ_ICM:
			cmd_execute = cmd_read_icm;
			break;
		case CMD_ICM_DATA_GOT:
			cmd_execute = cmd_icm_data_got;
			break;
		case CMD_ICM_NOTIF:
			cmd_execute = cmd_icm_data_notif;
			break;	
		case CMD_SET_GYRO_FSR:
			cmd_execute = cmd_set_gyro_fsr;
			break;
		case CMD_SET_ACC_FSR:
			cmd_execute = cmd_set_acc_fsr;
			break;
		case CMD_SET_GYRO_LPF:
			cmd_execute = cmd_set_gyro_lpf;
			break;
		case CMD_SET_ACC_LPF:
			cmd_execute = cmd_set_acc_lpf;
			break;
		case CMD_RESET:
			cmd_execute = cmd_reset;
			break;
		case CMD_DFU:
			cmd_execute = cmd_dfu;
			break;
		case CMD_READ_BATT:
			cmd_execute = cmd_read_batt;
			break;
		default:
			err_code = NRF_ERROR_NOT_SUPPORTED;
			cmd_execute = NULL;
			break;
	}
	
	if(cmd_execute != NULL) {
		err_code = cmd_execute(&cmd);
	}
	
	return err_code;
}

ret_code_t ctl_cmd_init(void)
{
	ctl_cmd_queue_init();

	return NRF_SUCCESS;
}

