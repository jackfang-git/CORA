#include <string.h>
#include "drv_xt25q32.h"
#include "flex_debug.h"
#include "nrf_gpio.h"
#include "drv_spi.h"
#include "nrf_delay.h"

#define	XT25Q_W_ENABLE				0x06
#define	XT25Q_W_DISABLE				0x04
#define	XT25Q_R_STATUS_REG			0x05
#define	XT25Q_W_STATUS_REG			0x01
#define	XT25Q_R_DATA				0x03
#define	XT25Q_FR_DATA				0x0B
#define	XT25Q_FR_DUAL				0x3B
#define	XT25Q_PAGE_PROG				0x02
#define	XT25Q_BLOCK_ERASE			0xD8
#define	XT25Q_SECTOR_ERASE			0x20
#define	XT25Q_CHIP_ERASE			0xC7
#define	XT25Q_POWER_DOWN			0xB9
#define	XT25Q_MANUFACTURE_ID		0x90
#define XT25Q_JEDEC_ID				0x9F
#define XT25Q_DEVICE_ID				0xAB
#define XT25Q_RELEASE_PD			0xAB


#define WRITE_IN_PROGRESS_FLAG		0x01
#define DUMMY_BYTE					0xFF

#define ERROR_NONE			0

#define FLASH_SPI_SELECT()			nrf_gpio_pin_clear(FLASH_NSS_PIN)
#define FLASH_SPI_DESELECT()		nrf_gpio_pin_set(FLASH_NSS_PIN)




static uint8_t drv_xt25q_read_status(void)
{
	uint8_t buf[2];

	buf[0] = XT25Q_R_STATUS_REG;
	buf[1] = DUMMY_BYTE;
	
	FLASH_SPI_SELECT();
	spi1_TxRx(buf,buf,2);
	FLASH_SPI_DESELECT();
	
	//flex_debug("\r\n read status reg:%x",buf[1]);
	
	return buf[1];
}
/*static void drv_xt25q_write_status(uint8_t value)
{
	uint8_t buf[2];

	buf[0] = XT25Q_W_STATUS_REG;
	buf[1] = value;
	
	FLASH_SPI_SELECT();
	spi1_TxRx(buf,NULL,2);
	FLASH_SPI_DESELECT();
}
*/
void drv_xt25q_wake_up(void)
{
	uint8_t buf;
	buf = XT25Q_RELEASE_PD;
	
	FLASH_SPI_SELECT();
	spi1_TxRx(&buf,NULL,1);
	FLASH_SPI_DESELECT();
}

uint32_t drv_xt25q_read_device_id(void)
{
	uint8_t err = 0;
	uint8_t buf[5];
	uint32_t id=0;
	
	memset(buf,0,5);
	buf[0] = XT25Q_DEVICE_ID;

	FLASH_SPI_SELECT();
	spi1_TxRx(buf,buf,5);
	FLASH_SPI_DESELECT();	
	
	if(err!=ERROR_NONE)
		flex_debug("\r\n r_err:%d",err);
	id = buf[4];
	//if(id!=0|id!=0xff)
	//	flex_debug("\r\n XT25Q ID:%x",id);

	return id;
}


uint32_t drv_xt25q_read_jedec_id(void)
{
	uint8_t err = 0;
	uint8_t buf[4];
	uint32_t id=0;
	
	memset(buf,0,4);
	buf[0] = XT25Q_JEDEC_ID;	
	
	FLASH_SPI_SELECT();
	nrf_delay_ms(10);
	err = spi1_TxRx(buf,buf,4); 
	FLASH_SPI_DESELECT();	
	
	if(err!=ERROR_NONE)
		flex_debug("\r\n r_err:%d",err);
	
	id = (buf[1] << 16) | (buf[2] << 8) | buf[3];
	//flex_debug("\r\n jedec id:%x",id);
	return id;
}

uint32_t drv_xt25q_read_manufacture_id(void)
{
	uint8_t err = 0;
	uint8_t buf[6];
	uint32_t id=0;
	
	memset(buf,0,6);
	buf[0] = XT25Q_MANUFACTURE_ID;	
	
	FLASH_SPI_SELECT();
	nrf_delay_ms(10);
	err = spi1_TxRx(buf,buf,6); 
	FLASH_SPI_DESELECT();	
	
	if(err!=ERROR_NONE)
		flex_debug("\r\n r_err:%d",err);
	id = (buf[4] << 8) | (buf[5]);
	//flex_debug("\r\n manufacture id:%x",id);
	return id;
}

void drv_xt25q_wait_for_write_end(void)
{
	uint8_t flash_status=0;
	uint32_t count = 0;
	do
	{
		count ++;
		flash_status = drv_xt25q_read_status();
	}
	while((flash_status&WRITE_IN_PROGRESS_FLAG) == true);
	//flex_debug("\r\n count :%d",count);
}

void drv_xt25q_write_enable(void)
{
	uint8_t buf = XT25Q_W_ENABLE;
	
	FLASH_SPI_SELECT();
	spi1_TxRx(&buf,NULL,1); 
	FLASH_SPI_DESELECT();
}

void drv_xt25q_page_write(uint32_t w_addr, uint8_t *buf, uint16_t len)
{
	uint8_t cmd;
	uint8_t addr_buf[3];

	cmd = XT25Q_PAGE_PROG;
	
	addr_buf[0] = (w_addr&0xFF0000)>>16;
	addr_buf[1] = (w_addr&0xFF00)>>8;
	addr_buf[2] = (w_addr&0xFF);
	
	drv_xt25q_write_enable();
	
	FLASH_SPI_SELECT();
	//send cmd
	spi1_TxRx(&cmd,NULL,1); 
	//send addr
	spi1_TxRx(addr_buf,NULL,3); 
	//send data
	if(len>XT25Q_PAGE_SIZE)
		len = XT25Q_PAGE_SIZE;
	while(len--)
	{
		spi1_TxRx(buf,NULL,1);
		buf++;
	}	
	FLASH_SPI_DESELECT();
	drv_xt25q_wait_for_write_end();
}

void drv_xt25q_buf_write(uint32_t w_addr, uint8_t *buf, uint32_t len)
{
	uint32_t num_page 		= 0;
	uint8_t num_single 		= 0;
	uint8_t aligned_addr 	= 0;
	uint8_t aligned_h_count = 0;
	uint8_t aligned_t_count = 0;
	
	aligned_addr 	= w_addr % XT25Q_PAGE_SIZE;
	aligned_h_count = XT25Q_PAGE_SIZE - aligned_addr;
	num_page 		= len / XT25Q_PAGE_SIZE;
	num_single 		= len % XT25Q_PAGE_SIZE;
	
	if(aligned_addr==0)
	{
		if(num_page == 0)
		{
			drv_xt25q_page_write(w_addr,buf,len);
		}
		else
		{
			while(num_page--)
			{
				drv_xt25q_page_write(w_addr,buf,XT25Q_PAGE_SIZE);
				w_addr  += XT25Q_PAGE_SIZE;
				buf 	+= XT25Q_PAGE_SIZE;
			}
			drv_xt25q_page_write(w_addr,buf,num_single);
		}
	}
	else
	{
		if(num_page == 0)
		{
			if(num_single > aligned_h_count) //len + w_addr > XT25Q_PAGE_SIZE
			{
				drv_xt25q_page_write(w_addr,buf,aligned_h_count);
				aligned_t_count = num_single - aligned_h_count;
				w_addr 	+= aligned_h_count;
				buf		+= aligned_h_count;
				drv_xt25q_page_write(w_addr,buf,aligned_t_count);
			}
			else
			{
				drv_xt25q_page_write(w_addr,buf,len);
			}
		}
		else
		{
			drv_xt25q_page_write(w_addr,buf,aligned_h_count);
			
			len 		-= aligned_h_count;
			num_page 	= len / XT25Q_PAGE_SIZE;
			num_single 	= len % XT25Q_PAGE_SIZE;
			w_addr 		+= aligned_h_count;
			buf 		+= aligned_h_count;
			while(num_page--)
			{
				drv_xt25q_page_write(w_addr,buf,XT25Q_PAGE_SIZE);
				w_addr	+= XT25Q_PAGE_SIZE;
				buf		+= XT25Q_PAGE_SIZE;
			}
			if(num_single != 0)
			{
				drv_xt25q_page_write(w_addr,buf,num_single);
			}
		}
	}	
}

void drv_xt25q_buf_read(uint32_t r_addr, uint8_t *buf, uint32_t len)
{
	uint8_t cmd;
	uint8_t addr_buf[3];	
	
	cmd = XT25Q_R_DATA;
	
	addr_buf[0] = (r_addr&0xFF0000)>>16;
	addr_buf[1] = (r_addr&0xFF00)>>8;
	addr_buf[2] = (r_addr&0xFF);
	
	drv_xt25q_write_enable();
	
	FLASH_SPI_SELECT();
	spi1_TxRx(&cmd,NULL,1); 
	//send addr
	spi1_TxRx(addr_buf,NULL,3); 
	//read data
	while(len--)
	{
		spi1_TxRx(NULL,buf,1);
		buf++;
	}	
	FLASH_SPI_DESELECT();	
}

void drv_xt25q_sector_erase(uint32_t addr)
{
	uint8_t cmd;
	uint8_t addr_buf[3];

	cmd = XT25Q_SECTOR_ERASE;
	
	addr_buf[0] = (addr&0xFF0000)>>16;
	addr_buf[1] = (addr&0xFF00)>>8;
	addr_buf[2] = (addr&0xFF);
	
	drv_xt25q_write_enable();
	drv_xt25q_wait_for_write_end();
	
	FLASH_SPI_SELECT();
	//send cmd
	spi1_TxRx(&cmd,NULL,1); 
	//send addr
	spi1_TxRx(addr_buf,NULL,3);  
	FLASH_SPI_DESELECT();
	//flex_debug("\r\n wait sector erase");
	drv_xt25q_wait_for_write_end();
}

void drv_xt25q_chip_erase(void)
{
	uint8_t cmd;

	cmd = XT25Q_CHIP_ERASE;
	
	drv_xt25q_write_enable();
	
	FLASH_SPI_SELECT();
	spi1_TxRx(&cmd,NULL,1); 
	FLASH_SPI_DESELECT();
	//flex_debug("\r\n wait sector erase");
	drv_xt25q_wait_for_write_end();
}

void drv_xt25q_init(void)
{
	uint32_t device_ID = 0;
	uint32_t jedec_ID = 0;
	uint32_t manu_ID = 0;
	
	spi1_init();
	nrf_delay_ms(10);
	drv_xt25q_read_status();
	device_ID	= drv_xt25q_read_device_id();
	jedec_ID 	= drv_xt25q_read_jedec_id();
	manu_ID 	= drv_xt25q_read_manufacture_id();
	//drv_xt25q_chip_erase();
	flex_debug("\r\n %x,%x,%x",device_ID,jedec_ID,manu_ID);
	//while(1);
}

void drv_xt25q_test(void)
{
	uint32_t addr 	= 1;
	uint32_t read 	= 0;
	uint32_t write 	= 0x12121212;	
	
	//drv_xt25q_chip_erase();
	drv_xt25q_sector_erase(addr);
	drv_xt25q_buf_read(addr,(uint8_t *)&read,4);
	flex_debug("\r\n ------read_1: %x",read);
	drv_xt25q_sector_erase(addr);
	drv_xt25q_buf_write(addr,(uint8_t *)&write,4);
	drv_xt25q_buf_read(addr,(uint8_t *)&read,4);
	flex_debug("\r\n ------read_2: %x",read);	
}
