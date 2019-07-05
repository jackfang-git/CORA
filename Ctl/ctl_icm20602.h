#ifndef CTL_ICM20602_H_
#define CTL_ICM20602_H_

#include <stdint.h>

void ctl_icm20602_read_cycle(uint32_t data);
void ctl_icm20602_read_enable(void);
void ctl_icm20602_read_disable(void);
#endif
