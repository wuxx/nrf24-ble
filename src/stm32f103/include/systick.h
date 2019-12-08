#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "stm32f10x.h"

#define SYSTICK_FREQ_US	(100000)	/* 100ms per systick interrupt */

extern volatile uint64_t systick;

void SysTick_Init(void);
void udelay(volatile uint32_t tus);         // µ¥Î»1us
void mdelay(volatile uint32_t tms);
uint64_t get_systick(void);

#endif /* __SYSTICK_H__ */
