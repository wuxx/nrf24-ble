#include <stdlib.h>
#include "log.h"
#include "gpio.h"
#include "timer.h"
#include "systest.h"

int32_t test_timer_all()
{
	uint32_t index;
	uint32_t arg1;
	uint64_t systime;

	PRINT_EMG("enter %s-%d \n", __func__, __LINE__);

	index = strtoul(argv[2], NULL, 0);
	arg1  = strtoul(argv[3], NULL, 0);
	PRINT_EMG("index: %d; arg1: %d\n", index, arg1);

	switch (index) {
		
		case (0):
			PRINT_EMG("systick reg value: 0x%X\n", SysTick->VAL);
			break;
		
		case (1):
			systime = get_time();
			PRINT_EMG("systime:   0x%X 0x%X\n", (uint32_t)(systime >> 32), (uint32_t)systime);
			break;
		
		case (10): /* NOTICE: run in int context or thread context ? */
			PRINT_EMG("before mdelay %d\n", arg1);
			mdelay(arg1);
			PRINT_EMG("after  mdelay %d\n", arg1);
			break;
		
		default:
			break;
	}


	return 0;
}

