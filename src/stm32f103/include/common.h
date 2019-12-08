#ifndef __COMMON_H__
#define __COMMON_H__

#include "stm32f10x.h"

#if 0 /* already defined in stm32f10x.h */
#undef SRAM_BASE
#undef FLASH_BASE
#define SRAM_BASE   (0x20000000)
#define FLASH_BASE  (0x08000000)
#endif

/* stm32f103c8t6 */
#define SRAM_SIZE   (20 * 1024)
#define FLASH_SIZE  (64 * 1024)


#define IS_FLASHADDR(addr) (((addr) >= FLASH_BASE) && ((addr) < (FLASH_BASE + FLASH_SIZE)))
#define IS_SRAMADDR(addr)  (((addr) >= SRAM_BASE)  && ((addr) < (SRAM_BASE  + SRAM_SIZE )))

#define IS_MEMADDR(addr)  (IS_FLASHADDR(addr) || IS_SRAMADDR(addr))

enum IO_TYPE_E
{
	IO_UART = 1 << 0,
	IO_USB  = 1 << 1,
};
extern uint32_t io_type;

int __assert(const char *file_name, const char *func_name, unsigned int line_num, char *desc);

#define ASSERT(exp) ((exp) ? 0 : __assert(__FILE__, __func__, __LINE__, "assert (" #exp ") failed!\n"))

#endif /* __COMMON_H__ */
