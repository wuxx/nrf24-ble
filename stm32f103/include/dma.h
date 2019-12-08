#ifndef __DMA_H__
#define __DMA_H__

#include "stm32f10x.h"

// 当使用存储器到存储器模式时候，通道可以随便选，没有硬性的规定
#define DMA_CHANNEL     DMA1_Channel6
#define DMA_CLOCK       RCC_AHBPeriph_DMA1

// 传输完成标志
#define DMA_FLAG_TC     DMA1_FLAG_TC6

int32_t dma_memcpy(void *dst, void *src, uint32_t size);

#endif /* __DMA_H__ */
