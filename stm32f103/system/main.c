/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include "config.h"
#include "common.h"
#include "int.h"
#include "mmio.h"
#include "uart.h"
#include "shell.h"
#include "log.h"
#include "gpio.h"
#include "timer.h"
#include "systick.h"

#include "hw_config.h"

extern unsigned char  Load$$ER_IROM1$$Base;
extern unsigned char Image$$ER_IROM1$$Base;
extern unsigned char Image$$ER_IROM1$$Length;

extern unsigned char  Load$$RW_IRAM1$$Base;
extern unsigned char Image$$RW_IRAM1$$Base;
extern unsigned char Image$$RW_IRAM1$$Length;

#define USB_IO_SIZE	(256)
uint32_t usb_recv_buf_index = 0;
char usb_recv_buf[USB_IO_SIZE] = {0};

uint32_t io_type = IO_UART;

uint32_t flash_base = FLASH_BASE, flash_size = FLASH_SIZE;
uint32_t ram_base = SRAM_BASE, ram_size = SRAM_SIZE;

uint32_t flash_load_base, flash_image_base, flash_image_size;
uint32_t ram_load_base, ram_image_base, ram_image_size;

uint32_t bss_image_base, bss_image_size;

char sys_banner[] = {"utos system buildtime [" __TIME__ " " __DATE__ "] " "rev " XXXX_REV};


void usb_vcom_handler(uint8_t ch)
{

		if (ch == '\n') {   /* sscom will send '\r\n' we ignore the '\n' */
				return;
		}
		
		if (usb_recv_buf_index == (USB_IO_SIZE - 1) && ch != '\r') {
				printf("cmd too long!\n");
				usb_recv_buf_index = 0;
				return;
		}

		if (ch == '\r') {
				usb_recv_buf[usb_recv_buf_index] = '\0';  /* terminate the string. */
				shell(usb_recv_buf);

				usb_recv_buf_index = 0;
				return;
		} else {
				usb_recv_buf[usb_recv_buf_index] = ch;
				usb_recv_buf_index++;
		}
		
		/* echo */
		printf("%c", ch);

}

void print_chipid()
{
	PRINT_EMG("chipid: %X%X%X\n",
					 __REV(readl(0X1FFFF7E8)), __REV(readl(0X1FFFF7EC)), __REV(readl(0X1FFFF7F0)));
}

/*
 * main: initialize and start the system
 */
int main (void)
{
	__local_irq_enable();
	uart_init();	
	timer_init();
	SysTick_Init();

//#define CONFIG_USB

#ifdef CONFIG_USB
	USB_Config();
	io_type |= IO_USB;
#endif
	
	/* 发送一个字符串 */
	PRINT_EMG("\n%s\n", sys_banner);
	print_chipid();
	PRINT_EMG("io_type [0x%08x]: %d\n", &io_type, io_type);
	PRINT_EMG("uart_baudrate %d\n", DEBUG_USART_BAUDRATE);
	
	flash_load_base  = (uint32_t)&Load$$ER_IROM1$$Base;
	flash_image_base = (uint32_t)&Image$$ER_IROM1$$Base;
	flash_image_size = (uint32_t)&Image$$ER_IROM1$$Length;

	ram_load_base  = (uint32_t)&Load$$RW_IRAM1$$Base;
	ram_image_base = (uint32_t)&Image$$RW_IRAM1$$Base;
	ram_image_size = (uint32_t)&Image$$RW_IRAM1$$Length;

	bss_image_base = ram_image_base + ram_image_size;
	bss_image_size = readl(ram_load_base - 8);	/* just binary guess. */
	
	PRINT_EMG("flash memory [0x%08x, 0x%08x]\n", flash_base, flash_base + flash_size);
	PRINT_EMG("ram   memory [0x%08x, 0x%08x]\n", ram_base, ram_base + ram_size);

	PRINT_EMG("flash image [0x%08x, 0x%08x] ((text & rodata & data))\n", flash_image_base, flash_image_base + flash_image_size);
	DUMP_VAR4(flash_load_base);
	DUMP_VAR4(flash_image_base);
	DUMP_VAR4(flash_image_size);
	
	PRINT_EMG("data image [0x%08x, 0x%08x]\n", ram_image_base, ram_image_base + ram_image_size);
	DUMP_VAR4(ram_load_base);
	DUMP_VAR4(ram_image_base);
	DUMP_VAR4(ram_image_size);
	
	PRINT_EMG("bss image [0x%08x, 0x%08x]\n", bss_image_base, bss_image_base + bss_image_size);
	DUMP_VAR4(bss_image_base);
	DUMP_VAR4(bss_image_size);
	
	NRF24L01_Init();
	while(NRF24L01_Check())
	{
		PRINT_EMG("nrf24l01 probe err\r\n");
		mdelay(500);
	}
	PRINT_EMG("nrf24l01 probe ok\r\n");

	nrf24l01_ble_broadcast();
	
	while(1);
	
	{
		
		while (1) {
				if (shell_cmd != NULL) {
					shell((char *)shell_cmd);
					shell_cmd = NULL;				
				}

#ifdef CONFIG_USB
				{
					uint32_t i, len;
					uint8_t ch;

					len = USB_RxRead(&ch, 1);
					if (len == 1) {
						usb_vcom_handler(ch);
					}
#if 0					
					for(i = 0; i < len; i++) {
							PRINT_EMG("usb read [0x%x][%c]\n", buf[i], buf[i]);
					}
					if (len > 0)
					{
							USB_TxWrite(buf, len);
					}
#endif
				}				
#endif	/* CONFIG_USB */
		}	
	}
}


int fputc(int ch, FILE *f)
{
	
	uint8_t c = '\r';
	
	if (io_type & IO_UART) {
		Usart_SendByte(DEBUG_USARTx, (uint8_t) ch);
	}

	if (io_type & IO_USB) {
		
		if (ch == '\n') {
			USB_TxWrite(&c, 1);
		}
		
		USB_TxWrite((uint8_t *)&ch, 1);
	}

		return (ch);
}

#if 0
///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}
#endif

