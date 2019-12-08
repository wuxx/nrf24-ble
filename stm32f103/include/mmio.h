#ifndef __MMIO_H__
#define __MMIO_H__

#include "stm32f10x.h"

#define readl(addr)         (*( ( volatile uint32_t * )(addr)) )
#define writel(addr, data)  (*( ( volatile uint32_t * )(addr)) = data)

#endif /* __MMIO_H__ */
