#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include "stm32f10x.h"

void IWDG_Feed(void);
void IWDG_Config(uint8_t prv ,uint16_t rlv);
void watchdog_reset(void);

#endif /* __WATCHDOG_H__ */
