#ifndef DRV_XT25Q_H_
#define DRV_XT25Q_H_

#include <stdint.h>

#define XT25Q_PAGE_SIZE				256
#define	XT25Q_SECTOR_SIZE			(XT25Q_PAGE_SIZE*16)			// 4K
#define XT25Q_BLOCK_SIZE			(XT25Q_SECTOR_SIZE*16)			// 64K
#define	XT25Q_CHIP_SIZE				(XT25Q_BLOCK_SIZE*512)			// 32M
#define	XT25Q_CHIP_PAGE				(512*16*16)						// 131072 page

//uint8_t drv_xt25q32_send_byte(uint8_t byte);
//uint8_t drv_xt25q32_read_byte(void);

void drv_xt25q_init(void);
void drv_xt25q_wait_for_write_end(void);
void drv_xt25q_page_write(uint32_t w_addr, uint8_t *buf, uint16_t len);
void drv_xt25q_buf_write(uint32_t w_addr, uint8_t *buf, uint32_t len);
void drv_xt25q_buf_read(uint32_t r_addr, uint8_t *buf, uint32_t len);
void drv_xt25q_sector_erase(uint32_t addr);
void drv_xt25q_chip_erase(void);


uint32_t drv_xt25q_read_jedec_id(void);
uint32_t drv_xt25q_read_device_id(void);
uint32_t drv_xt25q_read_manufacture_id(void);

void drv_xt25q_test(void);














#endif
