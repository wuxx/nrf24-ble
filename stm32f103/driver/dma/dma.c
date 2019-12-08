#include "timer.h"
#include "dma.h"

int32_t dma_memcpy(void *dst, void *src, uint32_t size)
{
		uint32_t count = 0;
	
	  DMA_InitTypeDef DMA_InitStructure;
	
		// 开启DMA时钟
		RCC_AHBPeriphClockCmd(DMA_CLOCK, ENABLE);
		// 源数据地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)src;
		// 目标地址
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dst;
		// 方向：外设到存储器（这里的外设是内部的FLASH）	
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		// 传输大小	
		DMA_InitStructure.DMA_BufferSize = size;
		// 外设（内部的FLASH）地址递增	    
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
		// 内存地址递增
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		// 外设数据单位	
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		// 内存数据单位
		DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;	 
		// DMA模式，一次或者循环模式
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
		//DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
		// 优先级：高	
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		// 使能内存到内存的传输
		DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
		// 配置DMA通道		   
		DMA_Init(DMA_CHANNEL, &DMA_InitStructure);
    //清除DMA数据流传输完成标志位
    DMA_ClearFlag(DMA_FLAG_TC);
		// 使能DMA
		DMA_Cmd(DMA_CHANNEL,ENABLE);
		
		/* block dma TODO: non-block dma */
		while(DMA_GetFlagStatus(DMA_FLAG_TC) == RESET) {
			mdelay(1);
			count++;
			
			if (count == 100) {
				return -1;
			}
		}
		
		return 0;
}

