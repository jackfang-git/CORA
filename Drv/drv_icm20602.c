#include "drv_icm20602.h"
#include "flex_debug.h"
#include "ctl_common.h"
#include "nrf_gpio.h"
#include "drv_spi.h"
#include "nrf_delay.h"
#include <string.h>

#define ICM_SPI_SELECT()		nrf_gpio_pin_clear(ICM_NSS_PIN)
#define ICM_SPI_DESELECT()		nrf_gpio_pin_set(ICM_NSS_PIN)



#define ERROR_NONE			0
#define ICM_WRITE_MODE		0
#define ICM_READ_MODE		0x80

uint8_t icm20602WriteRegister(uint8_t reg,uint8_t data)
{
	int8_t err=0;
	uint8_t buf[2];
	
	buf[0] = reg|ICM_WRITE_MODE;
	buf[1] = data;
	
	ICM_SPI_SELECT();
	err = spi0_TxRx(buf,NULL,2);
	ICM_SPI_DESELECT();
	if(err!=ERROR_NONE)
		flex_debug("\r\n w_err:%d",err);

    return err;
}

int8_t icm20602ReadRegister(uint8_t reg, uint8_t *data)
{
	int8_t err=0;
	uint8_t buf[2];
	
	buf[0] = reg|ICM_READ_MODE;
	buf[1] = 0;
    ICM_SPI_SELECT();
	err = spi0_TxRx(buf,buf,2); 
	ICM_SPI_DESELECT();
	if(err!=ERROR_NONE)
		flex_debug("\r\n r_err:%d",err);
	*data = buf[1];
    return err;
}

int8_t icm20602ReadMultpRegister(uint8_t reg, uint8_t *data, uint8_t length)
{
	int8_t err=0;
	uint8_t firstByte = reg|ICM_READ_MODE;
	
	ICM_SPI_SELECT();
	err = spi0_TxRx(&firstByte,&firstByte,1); 
	
	if(err!=ERROR_NONE)
	{
		flex_debug("\r\n err:%d",err);
		goto out;
	}
	err = spi0_TxRx(data,data,length); 
out:
	ICM_SPI_DESELECT();
	return err;
}

static uint8_t ICM_Set_Gyro_Fsr(uint8_t fsr)
{
	return icm20602WriteRegister(GYRO_CONFIG,fsr<<3); 
}

static uint8_t ICM_Set_Accel_Fsr(uint8_t fsr)
{
	return icm20602WriteRegister(ACCEL_CONFIG,fsr<<3); 
}

static uint8_t ICM_Acc_Set_LPF(uint16_t lpf)
{
	uint8_t data=0;
	if(lpf>=218)data=7;
	else if(lpf>=99)data=2;
	else if(lpf>=44)data=3;
	else if(lpf>=21)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return icm20602WriteRegister(ACCEL_CONFIG2,data);
}

static uint8_t ICM_Gyro_Set_LPF(uint16_t lpf)
{
	uint8_t data=0;
	if(lpf>=250)data=7;
	else if(lpf>=176)data=1;
	else if(lpf>=92)data=2;
	else if(lpf>=41)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6;  
	return icm20602WriteRegister(ICM_CONFIG,data); 
}

static uint8_t ICM_Set_LPF(uint16_t lpf)
{
	ICM_Gyro_Set_LPF(lpf/2);	
	ICM_Acc_Set_LPF(lpf/2);
	return 0;
}

static uint8_t ICM_Set_Rate(uint16_t rate)
{
	uint8_t data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=icm20602WriteRegister(SMPLRT_DIV,data);	
 	return ICM_Set_LPF(rate);
}
#define NEW_A
#ifdef NEW_A
static void Get_xAverage(uint8_t times,uint16_t *a,uint8_t type)
{
	//short *x,*y,*z;
	short x[10],y[10],z[10];
	uint8_t t;
	
	//flex_debug("\r\n");
	if(type==GYRO)
	{
		//flex_debug("\r\n GYRO\r\n");
		for(t=0;t<times;t++)
		{
			ICM_Get_Gyroscope(&x[t],&y[t],&z[t]);
			//flex_debug("%4x",x[t]);
			a[t]=x[t];
			nrf_delay_ms(20);
		}
	}else if(type==ICM_ACC)
	{
		//flex_debug("\r\n ICM_ACC\r\n");
		for(t=0;t<times;t++)
		{
			ICM_Get_Accelerometer(&x[t],&y[t],&z[t]);
			//flex_debug("%x ",x[t]);
			a[t]=x[t];
			nrf_delay_ms(2);
		}
	}
	flex_debug("\r\nGet_xAverage\r\n");
	print_hex((uint8_t *)x,10*2);
	nrf_delay_ms(100);
}

static void Get_yAverage(uint8_t times,uint16_t *b,uint8_t type)
{
	//short *x,*y,*z;
	short x[10],y[10],z[10];
	uint8_t t;
	if(type==GYRO)
	{
		for(t=0;t<times;t++)
		{
			ICM_Get_Gyroscope(&x[t],&y[t],&z[t]);
			b[t]=y[t];
			nrf_delay_ms(2);
		}
	}else if(type==ICM_ACC)
	{
		for(t=0;t<times;t++)
		{
			ICM_Get_Accelerometer(&x[t],&y[t],&z[t]);
			b[t]=y[t];
			nrf_delay_ms(2);
		}
	}
}

static void Get_zAverage(uint8_t times,uint16_t *c,uint8_t type)
{
	//short *x,*y,*z;
	short x[10],y[10],z[10];
	uint8_t t;
	if(type==GYRO)
	{
		for(t=0;t<times;t++)
		{
			ICM_Get_Gyroscope(&x[t],&y[t],&z[t]);
			c[t]=z[t];
			nrf_delay_ms(2);
		}
	}else if(type==ICM_ACC)
	{
		for(t=0;t<times;t++)
		{
			ICM_Get_Accelerometer(&x[t],&y[t],&z[t]);
			c[t]=z[t];
			nrf_delay_ms(2);
		}
	}
}
#else
static void Get_xAverage(uint8_t times,uint16_t *a,uint8_t type)
{
	short *x,*y,*z;
	//short x[10],y[10],z[10];
	uint8_t t;
	if(type==GYRO)
	{
		for(t=0;t<times;t++)
		{
			x=x+t;
			ICM_Get_Gyroscope(x,y,z);
			a[t]=*x;
			nrf_delay_ms(2);
		}
	}else if(type==ICM_ACC)
	{
		for(t=0;t<times;t++)
		{
			x=x+t;
			ICM_Get_Accelerometer(x,y,z);
			a[t]=*x;
			nrf_delay_ms(2);
		}
	}
	print_hex((uint8_t *)a,10*2);
}

static void Get_yAverage(uint8_t times,uint16_t *b,uint8_t type)
{
	short *x,*y,*z;
	uint8_t t;
	if(type==GYRO)
	{
		for(t=0;t<times;t++)
		{
			y=y+t;
			ICM_Get_Gyroscope(x,y,z);
			b[t]=*y;
			nrf_delay_ms(2);
		}
	}else if(type==ICM_ACC)
	{
		for(t=0;t<times;t++)
		{
			y=y+t;
			ICM_Get_Accelerometer(x,y,z);
			b[t]=*y;
			nrf_delay_ms(2);
		}
	}
}

static void Get_zAverage(uint8_t times,uint16_t *c,uint8_t type)
{
	short *x,*y,*z;
	uint8_t t;
	if(type==GYRO)
	{
		for(t=0;t<times;t++)
		{
			z=z+t;
			ICM_Get_Gyroscope(x,y,z);
			c[t]=*z;
			nrf_delay_ms(2);
		}
	}else if(type==ICM_ACC)
	{
		for(t=0;t<times;t++)
		{
			z=z+t;
			ICM_Get_Accelerometer(x,y,z);
			c[t]=*z;
			nrf_delay_ms(2);
		}
	}
}
#endif
static void icm_Min2Max(uint16_t *x,uint8_t numbers)
{
	uint16_t temp;
	for(int n=0;n<numbers-1;n++)
	{
		for(int i=0;i<numbers-1;i++)
		{
			if(x[i]>x[i+1])
			{
				temp=x[i];
				x[i]=x[i+1];
				x[i+1]=temp;
			}
		}
	}
}

static uint16_t icm_Get_Average(uint16_t *x,uint8_t times)
{
	uint16_t temp=0;
	for(int i=0;i<times-4;i++)
	{
		temp+=x[2+i];
	}
	temp=temp/(times-4);
	return temp;
}

static uint8_t icm_Compare(uint16_t *x,uint8_t times)
{
	float res;
	uint16_t average;
	average=icm_Get_Average(x,times);
	res=((double)(x[times-3]-x[2]))/average;
	if(res>0.05f) return 0;
	else return 1;
}

uint8_t icm20602SpiDetect(void)
{
    uint8_t tmp;
	
	icm20602ReadRegister(WHO_AM_I, &tmp);
	flex_debug("\r\n ICM ID is %x\r\n",tmp);
	
	if (tmp == ICM20602_WHO_AM_I_CONST) 
	{
		return true;
	}
	return false;
}

int16_t ICM_Get_Temperature(void)
{
    uint8_t buf[2]={0,0}; 
    short raw;
	float temp;
	icm20602ReadMultpRegister(TEMP_OUT_H,buf,2);  
    raw=((uint16_t)buf[0]<<8)|buf[1];  
    temp=36.53+((double)raw)/340;  
    return temp*100;
}

uint8_t ICM_Get_Gyroscope(short *gx,short *gy,short *gz)
{
    uint8_t buf[6],res;  
	memset(buf,0,6);
	res=icm20602ReadMultpRegister(GYRO_XOUT_H,buf,6);
	if(res==ERROR_NONE)
	{
		*gx=((uint16_t)buf[0]<<8)|buf[1];  
		*gy=((uint16_t)buf[2]<<8)|buf[3];  
		*gz=((uint16_t)buf[4]<<8)|buf[5];
	} 	
    return res;
}

uint8_t ICM_Get_Accelerometer(short *ax,short *ay,short *az)
{
	uint8_t buf[6],res;
	memset(buf,0,6);
	res=icm20602ReadMultpRegister(ACCEL_XOUT_H,buf,6);
	if(res==ERROR_NONE)
	{
		*ax=((uint16_t)buf[0]<<8)|buf[1];
		*ay=((uint16_t)buf[2]<<8)|buf[3];
		*az=((uint16_t)buf[4]<<8)|buf[5];
	}
	return res;
}


uint8_t Acc_SelfTest(void)
{
	//uint16_t *x,*y,*z;
	uint16_t x[10],y[10],z[10];
	
	
	
	Get_xAverage(10,x,ICM_ACC);
	Get_yAverage(10,y,ICM_ACC);
	Get_zAverage(10,z,ICM_ACC);
	
	//print_hex((uint8_t *)x,10*2);
	//print_hex((uint8_t *)y,10*2);
	//print_hex((uint8_t *)z,10*2);
	
	icm_Min2Max(x,10);
	if(!icm_Compare(x,10)) return 0;
	
	icm_Min2Max(y,10);
	if(!icm_Compare(y,10)) return 0;
	
	icm_Min2Max(z,10);
	if(!icm_Compare(z,10)) return 0;

	return 1;
}

uint8_t Gyro_SelfTest(void)
{
	uint16_t *x,*y,*z;
	
	Get_xAverage(10,x,GYRO);
	Get_yAverage(10,y,GYRO);
	Get_zAverage(10,z,GYRO);
	
	icm_Min2Max(x,10);
	if(!icm_Compare(x,10)) return 0;
	
	icm_Min2Max(y,10);
	if(!icm_Compare(y,10)) return 0;
	
	icm_Min2Max(z,10);
	if(!icm_Compare(z,10)) return 0;

	return 1;
}

uint8_t Temp_SelfTest(void)
{
	uint16_t temp[10];
	for(int i=0;i<10;i++)
	{
		temp[i]=ICM_Get_Temperature();
	}
	icm_Min2Max(temp,10);
	if(!icm_Compare(temp,10)) return 0;
	else return 1;
}

void ICM_Print(void)
{
	int16_t g_buf[3];
	int16_t a_buf[3];
	memset(g_buf,0,3);
	memset(g_buf,0,3);
	while(1)
	{
		//flex_debug("\r\n\r\n Get Temp: %d\r\n",ICM_Get_Temperature());
		ICM_Get_Gyroscope(&g_buf[0],&g_buf[1],&g_buf[2]);
		//flex_debug("\r\n get Gyro:	      X     Y     Z");
		//print_hex((uint8_t *)&g_buf[0],sizeof(g_buf));
		ICM_Get_Accelerometer(&a_buf[0],&a_buf[1],&a_buf[2]);
		//flex_debug("\r\n get Acc:	      X     Y     Z");
		//print_hex((uint8_t *)&a_buf[0],sizeof(a_buf));
		
		nrf_delay_ms(2000);
	}
}



void drv_icm_set_gyro_fsr(uint8_t fsr)
{
	ICM_Set_Gyro_Fsr(fsr);
}
void drv_icm_set_acc_fsr(uint8_t fsr)
{
	ICM_Set_Accel_Fsr(fsr);
}
void drv_icm_set_gyro_lpf(uint16_t lpf)
{
	ICM_Gyro_Set_LPF(lpf);
}
void drv_icm_set_acc_lpf(uint16_t lpf)
{
	ICM_Acc_Set_LPF(lpf);
}

void drv_icm_switch(bool enable)
{
	icm20602WriteRegister(PWR_MGMT_1,enable);
}

uint8_t drv_icm_init(void)
{ 
	spi0_init();
	nrf_delay_ms(100);
	icm20602WriteRegister(PWR_MGMT_1,0X80);	
	nrf_delay_ms(100);
	icm20602WriteRegister(SIGNAL_PATH_RESET,0X03); //Reset accel digital signal path. Reset temp digital signal path.
	nrf_delay_ms(100);
//	icm20602WriteRegister(PWR_MGMT_1,0X00); 
	ICM_Set_Gyro_Fsr(0);					
	nrf_delay_ms(15);
	ICM_Set_Accel_Fsr(0);					
	nrf_delay_ms(15);
	ICM_Set_Rate(50);						
	nrf_delay_ms(15);
	icm20602WriteRegister(INT_PIN_CFG,0X10);
	nrf_delay_ms(15);
	icm20602WriteRegister(INT_ENABLE,0X00);	
	icm20602WriteRegister(PWR_MGMT_1,0X00); //ENABLE ICM
//	icm20602WriteRegister(FIFO_EN,0X00);	
//	icm20602WriteRegister(INT_PIN_CFG,0X80);	
	
	return icm20602SpiDetect();					
}

