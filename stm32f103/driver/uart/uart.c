#include <string.h>

#include "common.h"
#include "watchdog.h"
#include "uart.h"
#include "shell.h"
#include "log.h"

int uart_work_mode = TSHELL_MODE;

uint32_t uart_recv_buf_index = 0;
char uart_recv_buf[UART_IO_SIZE] = {0};

 /**
  * @brief  配置嵌套向量中断控制器NVIC
  * @param  无
  * @retval 无
  */
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* 嵌套向量中断控制器组选择 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* 配置USART为中断源 */
  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;
  /* 抢断优先级*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* 子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* 使能中断 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* 初始化配置NVIC */
  NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  USART GPIO 配置,工作参数配置
  * @param  无
  * @retval 无
  */
void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	DEBUG_USART_GPIO_APBxClkCmd(DEBUG_USART_GPIO_CLK, ENABLE);
	
	// 打开串口外设的时钟
	DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(DEBUG_USARTx, &USART_InitStructure);
	
	// 串口中断优先级配置
	NVIC_Configuration();
	
	// 使能串口接收中断
	//USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);	
	
	// 使能串口
	USART_Cmd(DEBUG_USARTx, ENABLE);		

  // 清除发送完成标志
	//USART_ClearFlag(USART1, USART_FLAG_TC);     
}

/*****************  发送一个字符 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	if (ch == '\n') {
		USART_SendData(pUSARTx, '\r');			
		while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	}
	
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/*****************  发送字符串 **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}

/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;
	/* 取出低八位 */
	temp_l = ch&0XFF;
	
	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

void DEBUG_USART_IRQHandler(void)
{
	static uint8_t i;
	static char magic_cmd[7] = {0};
	uint16_t ch;
	ch = (uint8_t)USART_ReceiveData(DEBUG_USARTx);

    for(i = 0; i < 5; i++) {
        magic_cmd[i] = magic_cmd[i + 1];
    }
    magic_cmd[5] = ch;

		if (strcmp(magic_cmd, "sreset") == 0) {
				uart_puts("magic_cmd: system reset!\n");
        watchdog_reset();
		}
		if (strcmp(magic_cmd, "tshell") == 0) {
			  uart_puts("magic_cmd: switch to thread shell!\n");        
				uart_work_mode = TSHELL_MODE;
		}
		if (strcmp(magic_cmd, "ishell") == 0) {
				uart_puts("magic_cmd: switch to int shell!\n");
				uart_work_mode = ISHELL_MODE;
		}

		if (ch == '\n') {   /* sscom will send '\r\n' we ignore the '\n' */
				return;
		}
		if (uart_recv_buf_index == (UART_IO_SIZE - 1) && ch != '\r') {
				uart_puts("cmd too long!\n");
				uart_recv_buf_index = 0;
				return;
		}

		if (ch == '\r') {
				uart_recv_buf[uart_recv_buf_index] = '\0';  /* terminate the string. */
				switch (uart_work_mode) {
					case (ISHELL_MODE):
						shell(uart_recv_buf);
						break;
					case (TSHELL_MODE):
						if (shell_cmd == NULL) {
							shell_cmd = uart_recv_buf;
						}
						break;
					default:
						break;
				}
				uart_recv_buf_index = 0;
				return;
		} else {
				uart_recv_buf[uart_recv_buf_index] = ch;
				uart_recv_buf_index++;
		}
		
		/* echo */
		printf("%c", ch);

}

void uart_putc(uint8_t byte)
{
	Usart_SendByte(DEBUG_USARTx, byte);
}

void uart_puts(char *s)
{
	Usart_SendString(DEBUG_USARTx, s);
}

void uart_init()
{
	USART_Config();
}
