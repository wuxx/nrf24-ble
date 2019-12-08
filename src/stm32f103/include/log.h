#ifndef __LOG_H__
#define __LOG_H__

#include "stm32f10x.h"
#include <stdio.h>

enum LOG_LEVEL_E {
    LOG_EMG = 1,
    LOG_ERR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_MAX,
};

extern uint32_t default_log_level;

uint32_t set_log_level(uint32_t log_level);

#define PRINT_EMG(fmt, ...)     do { if ( default_log_level >= LOG_EMG   ) { printf(fmt, ##__VA_ARGS__); } } while (0)
#define PRINT_ERR(fmt, ...)     do { if ( default_log_level >= LOG_ERR   ) { printf(fmt, ##__VA_ARGS__); } } while (0)
#define PRINT_WARN(fmt, ...)    do { if ( default_log_level >= LOG_WARN  ) { printf(fmt, ##__VA_ARGS__); } } while (0)
#define PRINT_INFO(fmt, ...)    do { if ( default_log_level >= LOG_INFO  ) { printf(fmt, ##__VA_ARGS__); } } while (0)
#define PRINT_DEBUG(fmt, ...)   do { if ( default_log_level >= LOG_DEBUG ) { printf(fmt, ##__VA_ARGS__); } } while (0)

#define PRINT_STAMP()           PRINT_EMG("%s:%s:%d\n", __FILE__, __func__, __LINE__)
#define DUMP_VAR4(var)          PRINT_EMG(#var":\t 0x%08x\n", var)

#endif /* __LOG_H__ */
