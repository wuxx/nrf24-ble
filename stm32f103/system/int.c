/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_it.h" 
#include "uart.h"
#include "log.h"
#include "watchdog.h"
#include "int.h"

#if 0
/* CPU CONTEXT */
#define R0		(0)
#define R1		(1)
#define R2		(2)
#define R3		(3)
#define R12		(4)
#define LR		(5)
#define PC		(6)
#define XPSR	(7)
#endif

struct cpu_context
{
    uint32_t R4;
    uint32_t R5;
    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
    uint32_t R14;

    /* cpu auto-save */
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;
    uint32_t R12;
    uint32_t LR;
    uint32_t PC;
    uint32_t XPSR;
};

void __local_irq_disable(void)
{
	__asm { cpsid i };
}

void __local_irq_enable(void)
{
	__asm { cpsie i };
}

void __NMI_Handler(void)
{
    PRINT_EMG("%s-%d \n", __func__, __LINE__);
    watchdog_reset();
}

void __HardFault_Handler(unsigned int *sp)
{
	//__asm ("push {r4-r11, r14}");
	//register unsigned int *sp __asm ("sp");

	struct cpu_context *cc;
	unsigned int mfsr, bfsr, ufsr;

	cc = (struct cpu_context *)sp;
	__local_irq_disable();
	
  PRINT_EMG("%s:\n", __func__);
	PRINT_EMG("XPSR: 0x%08X \n", cc->XPSR);
	PRINT_EMG("PC:   0x%08X \n", cc->PC);
	PRINT_EMG("LR:   0x%08X \n", cc->LR);
	PRINT_EMG("R12:  0x%08X \n", cc->R12);
	PRINT_EMG("R3:   0x%08X \n", cc->R3);
	PRINT_EMG("R2:   0x%08X \n", cc->R2);
	PRINT_EMG("R1:   0x%08X \n", cc->R1);
	PRINT_EMG("R0:   0x%08X \n", cc->R0);

	PRINT_EMG("R14:  0x%08X \n", cc->R14);
	PRINT_EMG("R11:  0x%08X \n", cc->R11);
	PRINT_EMG("R10:  0x%08X \n", cc->R10);
	PRINT_EMG("R9:   0x%08X \n", cc->R9);
	PRINT_EMG("R8:   0x%08X \n", cc->R8);
	PRINT_EMG("R7:   0x%08X \n", cc->R7);
	PRINT_EMG("R6:   0x%08X \n", cc->R6);
	PRINT_EMG("R5:   0x%08X \n", cc->R5);
	PRINT_EMG("R4:   0x%08X \n\n", cc->R4);

	PRINT_EMG("HFSR: 0x%08X \n", SCB->HFSR);
	PRINT_EMG("CFSR: 0x%08X \n", SCB->CFSR);
	
	mfsr = ((SCB->CFSR) & 0x000000FF);
	bfsr = ((SCB->CFSR) & 0x0000FF00) >> 8;
	ufsr = ((SCB->CFSR) & 0xFFFF0000) >> 16;
	PRINT_EMG("MFSR: 0x%08X \n", mfsr);
	PRINT_EMG("BFSR: 0x%08X \n", bfsr);
	PRINT_EMG("UFSR: 0x%08X \n", ufsr);
	
	PRINT_EMG("MMAR: 0x%08X \n", SCB->MMFAR);
	PRINT_EMG("BFAR: 0x%08X \n", SCB->BFAR);

	watchdog_reset();
	while (1)
	{
	}

}

void __MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    PRINT_EMG("%s-%d \n", __func__, __LINE__);
	
    watchdog_reset();
    while (1)
    {
    }
}
 
void __BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    PRINT_EMG("%s-%d \n", __func__, __LINE__);

    watchdog_reset();
    while (1)
    {
    }
}
 
void __UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    PRINT_EMG("%s-%d \n", __func__, __LINE__);

    watchdog_reset();
    while (1)
    {
    }
}

void __DebugMon_Handler(void)
{
  PRINT_EMG("%s-%d \n", __func__, __LINE__);
	while(1);

}
