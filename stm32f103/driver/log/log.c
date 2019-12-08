#include "log.h"

uint32_t default_log_level = LOG_INFO;

uint32_t set_log_level(uint32_t log_level)
{
		if (log_level >= LOG_EMG && log_level < LOG_MAX) {
			default_log_level = log_level;
		}
		return 0;
}
