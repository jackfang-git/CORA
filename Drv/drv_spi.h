#ifndef KPD_DRV_NFC_H_
#define KPD_DRV_NFC_H_
#include <stdint.h>
#include <stdbool.h>
#include "app_util_platform.h"
#include "nrf_delay.h"

//#define ICM_OLD_PIN

#ifndef  ICM_OLD_PIN
#define ICM_SCLK_PIN	11//15
#define ICM_MOSI_PIN    12//14
#define ICM_MISO_PIN    13//13
#define ICM_NSS_PIN     14//12
#else
#define ICM_SCLK_PIN	15
#define ICM_MOSI_PIN    14
#define ICM_MISO_PIN    13
#define ICM_NSS_PIN     12
#endif

#define FLASH_SCLK_PIN	8
#define FLASH_MOSI_PIN	7
#define FLASH_MISO_PIN	6
#define FLASH_NSS_PIN	5


//#define SPI1_GPIO	1

void spi0_init(void);
void spi0_uninit(void);
int8_t spi0_TxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint8_t length);

void spi1_init(void);
void spi1_uninit(void);
int8_t spi1_TxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint8_t length);
uint8_t spi1_rw(uint8_t w_data);
void delayNMilliSeconds(uint32_t ms);
//void AMS_MEMMOVE(void *dest, const void *src, uint32_t length);



#endif
