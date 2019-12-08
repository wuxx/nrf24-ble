
#include "timer.h"
#include "gpio.h"

/* 1ms counter */
volatile uint64_t time = 0;

#if 0
void udelay(uint32_t tick)
{
    volatile uint32_t t, i, x;

	for(t = 0; t < tick; t++) {
		for(i = 0; i < 1; i++) {

			for(x = 0; x < 10; x++) {
				__asm volatile ("nop");
			}
		}
	}
}
#endif

void mdelay(uint32_t tick)
{
    uint64_t old_time = time;

    tick = tick <= 1000000 ? tick : 1000000;

    while(1) {
        if ((time - old_time) >= tick) {
            break;
        }
    }

}

// 中断优先级配置
static void ADVANCE_TIM_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // 设置中断组为0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);		
		// 设置中断来源
    NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_IRQ ;	
		// 设置主优先级为 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	 
	  // 设置抢占优先级为3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

///*
// * 注意：TIM_TimeBaseInitTypeDef结构体里面有5个成员，TIM6和TIM7的寄存器里面只有
// * TIM_Prescaler和TIM_Period，所以使用TIM6和TIM7的时候只需初始化这两个成员即可，
// * 另外三个成员是通用定时器和高级定时器才有.
// *-----------------------------------------------------------------------------
// *typedef struct
// *{ TIM_Prescaler            都有
// *	TIM_CounterMode			     TIMx,x[6,7]没有，其他都有
// *  TIM_Period               都有
// *  TIM_ClockDivision        TIMx,x[6,7]没有，其他都有
// *  TIM_RepetitionCounter    TIMx,x[1,8,15,16,17]才有
// *}TIM_TimeBaseInitTypeDef; 
// *-----------------------------------------------------------------------------
// */
static void ADVANCE_TIM_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;		
		// 开启定时器时钟,即内部时钟CK_INT=72M
    ADVANCE_TIM_APBxClock_FUN(ADVANCE_TIM_CLK, ENABLE);	
		// 自动重装载寄存器的值，累计TIM_Period+1个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Period=ADVANCE_TIM_Period;
	  // 时钟预分频数
    TIM_TimeBaseStructure.TIM_Prescaler= ADVANCE_TIM_Prescaler;	
		// 时钟分频因子 ，没用到不用管
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		
		// 计数器计数模式，设置为向上计数
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 		
		// 重复计数器的值，没用到不用管
		TIM_TimeBaseStructure.TIM_RepetitionCounter=0;	
	  // 初始化定时器
    TIM_TimeBaseInit(ADVANCE_TIM, &TIM_TimeBaseStructure);
	
		// 清除计数器中断标志位
    TIM_ClearFlag(ADVANCE_TIM, TIM_FLAG_Update);
	  
		// 开启计数器中断
    TIM_ITConfig(ADVANCE_TIM,TIM_IT_Update,ENABLE);
		
		// 使能计数器
    TIM_Cmd(ADVANCE_TIM, ENABLE);
}

void  ADVANCE_TIM_IRQHandler (void)
{
    if ( TIM_GetITStatus( ADVANCE_TIM, TIM_IT_Update) != RESET ) 
    {   
        time++;
		
		/* led blink */
		if (time % 1000) {
			gpio_write(GROUPB, 1, (time / 1000) % 2);
		}
		
        TIM_ClearITPendingBit(ADVANCE_TIM , TIM_FLAG_Update);        
    }           
}

uint64_t get_time()
{
	return time;
}

void timer_init()
{
	/* the LED blink in timer irq handler */
	gpio_init(GROUPB, 1, GPIO_Mode_Out_PP);
	gpio_write(GROUPB, 1, 0);
	
	ADVANCE_TIM_NVIC_Config();
	ADVANCE_TIM_Mode_Config();		
}

