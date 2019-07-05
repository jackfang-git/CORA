#include "drv_spi.h"
#include "nrf_drv_spi.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "flex_debug.h"



#define SPI_INSTANCE0  0
static const nrf_drv_spi_t spi0 = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE0); 
static uint8_t spi0_init_flag = 0;

#define SPI_INSTANCE1  1
static const nrf_drv_spi_t spi1 = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE1); 
static uint8_t spi1_init_flag = 0;


int8_t spi0_TxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint8_t length)
{
    if(1==length)
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi0, txBuf, length, rxBuf, length));
    else
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi0, txBuf, length, rxBuf, length));
    return 0;
}

int8_t spi1_TxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint8_t length)
{
    if(1==length)
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi1, txBuf, length, rxBuf, length));
    else
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi1, txBuf, length, rxBuf, length));
    return 0;
}

uint8_t spi1_rw(uint8_t w_data)
{
	uint8_t r_data=0;
	//nrf_gpio_pin_clear(FLASH_NSS_PIN);
	
	// mode 0
	/*for(uint8_t i=0; i<8; i++)
	{
		nrf_gpio_pin_clear(FLASH_SCLK_PIN);
		r_data <<= 1;
		r_data |= nrf_gpio_pin_read(FLASH_MISO_PIN);
		if(w_data|0x80)
			nrf_gpio_pin_set(FLASH_MOSI_PIN);
		else
			nrf_gpio_pin_clear(FLASH_MOSI_PIN);
		nrf_gpio_pin_set(FLASH_SCLK_PIN);
		w_data <<= 1;
		//r_data <<= 1;
		//r_data |= nrf_gpio_pin_read(FLASH_MISO_PIN);
	}
	nrf_gpio_pin_clear(FLASH_SCLK_PIN);
	*/
	
	// mode 3
	for(uint8_t i=0; i<8; i++)
	{
		nrf_gpio_pin_clear(FLASH_SCLK_PIN);
		if(w_data|0x80)
			nrf_gpio_pin_set(FLASH_MOSI_PIN);
		else
			nrf_gpio_pin_clear(FLASH_MOSI_PIN);
		nrf_gpio_pin_set(FLASH_SCLK_PIN);
		w_data <<= 1;		
		r_data <<= 1;
		r_data |= nrf_gpio_pin_read(FLASH_MISO_PIN);
	}
	
	return r_data;
}

void delayNMilliSeconds(uint32_t ms)
{
    nrf_delay_ms(ms);
}
/*
void AMS_MEMMOVE(void *dest, const void *src, uint32_t length)
{
    uint8_t* chardest = (uint8_t*)dest;
    const uint8_t* charsrc = (uint8_t*)src;
    uint32_t i = 0;

    if (chardest < charsrc) while (i<length)
    {
        chardest[i] = charsrc[i];
        i++;
    }else while (length--)
    {
        chardest[length] = charsrc[length];
    }
}
*/

void spi0_init(void)
{
    if(spi0_init_flag == 1)
        return;
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.miso_pin = ICM_MISO_PIN;
    spi_config.mosi_pin = ICM_MOSI_PIN;
    spi_config.sck_pin  = ICM_SCLK_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi0, &spi_config, NULL));
    nrf_gpio_cfg_output(ICM_NSS_PIN);
    nrf_gpio_pin_set(ICM_NSS_PIN);
    
    spi0_init_flag = 1;
}
void spi0_uninit(void)
{
    if(spi0_init_flag == 0)
        return;
    nrf_drv_spi_uninit(&spi0);
    //nrf_spi_disable(spi.p_registers);
    nrf_gpio_pin_set(ICM_NSS_PIN);
    spi0_init_flag = 0;
}

void ctl_all(bool data)
{
	nrf_gpio_pin_write(FLASH_NSS_PIN,data);
	nrf_gpio_pin_write(FLASH_SCLK_PIN,data);
	nrf_gpio_pin_write(FLASH_MOSI_PIN,data);
	nrf_gpio_pin_write(FLASH_MISO_PIN,1);
	
}
void spi1_init(void)
{
    if(spi1_init_flag == 1)
        return;
#ifndef SPI1_GPIO
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.frequency= NRF_DRV_SPI_FREQ_4M;
	spi_config.mode 	= NRF_DRV_SPI_MODE_3;
    spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.miso_pin = FLASH_MISO_PIN;
    spi_config.mosi_pin = FLASH_MOSI_PIN;
    spi_config.sck_pin  = FLASH_SCLK_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi1, &spi_config, NULL));
    nrf_gpio_cfg_output(FLASH_NSS_PIN);
	//nrf_gpio_pin_clear(FLASH_NSS_PIN);
    nrf_gpio_pin_set(FLASH_NSS_PIN);
#else
	nrf_gpio_cfg_output(FLASH_SCLK_PIN);
	nrf_gpio_cfg_output(FLASH_MOSI_PIN);
	//nrf_gpio_cfg_output(FLASH_MISO_PIN);
	nrf_gpio_cfg_input(FLASH_MISO_PIN,NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_output(FLASH_NSS_PIN);
	nrf_gpio_pin_set(FLASH_NSS_PIN);
	//ctl_all(1);

	//while(1);
#endif	
    spi1_init_flag = 1;
}
void spi1_uninit(void)
{
    if(spi1_init_flag == 0)
        return;
    nrf_drv_spi_uninit(&spi1);
    //nrf_spi_disable(spi.p_registers);
    nrf_gpio_pin_set(FLASH_NSS_PIN);
    spi1_init_flag = 0;
}

