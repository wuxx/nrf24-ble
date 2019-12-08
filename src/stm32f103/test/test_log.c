#include <stdlib.h>
#include "systest.h"
#include "log.h"
#include "timer.h"
#include "log.h"

int32_t test_log_all()
{
	uint32_t index;
	uint32_t arg1;
		
	PRINT_EMG("enter %s-%d \n", __func__, __LINE__);

	index = strtoul(argv[2], NULL, 0);
	arg1  = strtoul(argv[3], NULL, 0);
	PRINT_EMG("index: %d; arg1: %d\n", index, arg1);

	switch (index) {		
		case (0):
			PRINT_EMG("%s-%d\n", __func__, __LINE__);
			break;
		case (1):
			PRINT_ERR("%s-%d\n", __func__, __LINE__);
			break;
		case (2):
			PRINT_WARN("%s-%d\n", __func__, __LINE__);
			break;
		case (3):
			PRINT_INFO("%s-%d\n", __func__, __LINE__);
			break;
		case (4):
			PRINT_DEBUG("%s-%d\n", __func__, __LINE__);
			break;
		default:
			break;
	}


	return 0;
}

