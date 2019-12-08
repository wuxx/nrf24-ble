#include <stdlib.h>
#include "log.h"
#include "gpio.h"
#include "timer.h"
#include "systest.h"

int32_t test_gpio_all()
{
	uint32_t index, tick;
	uint32_t gpio_group, gpio_index;
	uint32_t bit;
	

	PRINT_EMG("enter %s-%d \n", __func__, __LINE__);

	index = strtoul(argv[2], NULL, 0);
	PRINT_EMG("index: %d\n", index);

	switch (index) {
		case (0):
			gpio_group = strtoul(argv[3], NULL, 0);
			gpio_index = strtoul(argv[4], NULL, 0);
			PRINT_EMG("group: 0x%x; index: %d;\n", gpio_group, gpio_index);
			gpio_init (gpio_group, gpio_index, GPIO_Mode_Out_PP);
			//gpio_write(gpio_group, gpio_index, bit);
			break;
		case (1):
			gpio_init(0xB, 0x9, GPIO_Mode_Out_PP);
			tick = strtoul(argv[3], NULL, 0);
			PRINT_EMG("tick: %d\n", tick);

			while(1) {
				gpio_write(0xB, 0x9, 0);
				mdelay(tick);
				gpio_write(0xB, 0x9, 1);
				mdelay(tick);
			}
			break;
		case (10):
			gpio_group = strtoul(argv[3], NULL, 0);
			gpio_index = strtoul(argv[4], NULL, 0);
			PRINT_EMG("group: %d; index: %d;\n", gpio_group, gpio_index);
			bit = gpio_read(gpio_group, gpio_index);
			PRINT_EMG("read bit %d\n", bit);
			break;

		default:
			break;
	}

	return 0;
}

