#ifndef CTL_BLE_H_
#define CTL_BLE_H_

#include <stdint.h>

void ctl_ble_init(void);
void data_send(uint8_t *data, uint8_t len);
void multip_data_send(uint8_t *data, uint8_t len);
void data_read(uint8_t *data, uint8_t len);

#endif

