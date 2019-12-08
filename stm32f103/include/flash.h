#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"

/* stm32f103vet6 flash page: 2048 bytes */
/* stm32f103c8t6 flash page: 1024 bytes */
#define FLASH_PAGE_SIZE   ((uint16_t)0x400)	//1024

#define PAGE_BASE(addr)   ((addr) - ((addr) % FLASH_PAGE_SIZE))
#define PAGE_OFFSET(addr) ((addr) % FLASH_PAGE_SIZE)


int32_t flash_write(uint32_t addr, void *buf, uint32_t size);
int32_t flash_read(uint32_t addr, void *buf, uint32_t size);

#endif /* __FLASH_H__ */
