#include <stdlib.h>
#include "log.h"
#include "timer.h"
#include "dma.h"
#include "systest.h"

int32_t test_dma_all()
{
	uint32_t i;
	uint32_t index;
	uint32_t arg1;
	
	uint8_t bsrc[10];
	uint8_t bdst[10] = {0};
	
	PRINT_EMG("enter %s-%d \n", __func__, __LINE__);

	index = strtoul(argv[2], NULL, 0);
	arg1  = strtoul(argv[3], NULL, 0);
	PRINT_EMG("index: %d; arg1: %d\n", index, arg1);

	switch (index) {
		
		case (0):
			for(i = 0; i < sizeof(bsrc); i++) {
				bsrc[i] = i;
			}
			dma_memcpy(bdst, bsrc, sizeof(bsrc));

			for(i = 0; i < sizeof(bdst); i++) {
				PRINT_EMG("[%d]: 0x%02x\n", i, bdst[i]);
			}
			break;
		default:
			break;
	}


	return 0;
}

