#ifndef __INT_H__
#define __INT_H__

#include "stm32f10x.h"
#include <stdio.h>

void __local_irq_disable(void);
void __local_irq_enable (void);

#endif /* __INT_H__ */
