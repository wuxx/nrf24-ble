#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "systest.h"
#include "flash.h"
#include "log.h"

static uint8_t page[FLASH_PAGE_SIZE];

int32_t test_flash_all()
{
	uint32_t index;
	uint32_t arg1, arg2;
	

	
	PRINT_EMG("enter %s-%d \n", __func__, __LINE__);

	index = strtoul(argv[2], NULL, 0);
	arg1  = strtoul(argv[3], NULL, 0);
	arg2  = strtoul(argv[4], NULL, 0);
	PRINT_EMG("index: %d; arg1: 0x%08x; arg2: 0x%08x\n", index, arg1, arg2);

	switch (index) {

		/* flash memset */
		case (0):
			memset(page, arg2, sizeof(page));
			flash_write(arg1, page, sizeof(page));
			break;
		
		default:
			break;
	}

	return 0;
}

