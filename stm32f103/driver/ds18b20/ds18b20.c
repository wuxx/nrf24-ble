#include "int.h"
#include "ds18b20.h"
#include "log.h"
#include "systick.h"

static void                           DS18B20_GPIO_Config                       ( void );
static void                           DS18B20_Mode_IPU                          ( void );
static void                           DS18B20_Mode_Out_PP                       ( void );
static void                           DS18B20_Rst                               ( void );
static uint8_t                        DS18B20_Presence                          ( void );
static uint8_t                        DS18B20_ReadBit                           ( void );
static uint8_t                        DS18B20_ReadByte                          ( void );
static void                           DS18B20_WriteByte                         ( uint8_t dat );
static void                           DS18B20_SkipRom                           ( void );
static void                           DS18B20_MatchRom                          ( void );


uint8_t ds18b20_id[8];

 /**
  * @brief  DS18B20 初始化函数
  * @param  无
  * @retval 无
  */
uint8_t DS18B20_Init(void)
{
	uint8_t p;
	uint32_t *id;
	
	DS18B20_GPIO_Config ();
	
	DS18B20_DQ_1;
	
	DS18B20_Rst();

	p = DS18B20_Presence ();

	DS18B20_ReadId(ds18b20_id);

	id = (uint32_t *)ds18b20_id;
	
	PRINT_EMG("ds18b20 id: [%x%x]\n", __REV(id[0]), __REV(id[1]));
	
	PRINT_EMG("DS18B20_Init %s \n", p == 0 ? "succ" : "fail");

	return p;
}


/*
 * 函数名：DS18B20_GPIO_Config
 * 描述  ：配置DS18B20用到的I/O口
 * 输入  ：无
 * 输出  ：无
 */
static void DS18B20_GPIO_Config(void)
{		
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;
	

	/*开启DS18B20_DQ_GPIO_PORT的外设时钟*/
	DS18B20_DQ_SCK_APBxClock_FUN ( DS18B20_DQ_GPIO_CLK, ENABLE); 

	/*选择要控制的DS18B20_DQ_GPIO_PORT引脚*/															   
  	GPIO_InitStructure.GPIO_Pin = DS18B20_DQ_GPIO_PIN;	

	/*设置引脚模式为通用推挽输出*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*设置引脚速率为50MHz */   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*调用库函数，初始化DS18B20_DQ_GPIO_PORT*/
  	GPIO_Init ( DS18B20_DQ_GPIO_PORT , &GPIO_InitStructure );

}

/*
 * 函数名：DS18B20_Mode_IPU
 * 描述  ：使DS18B20-DATA引脚变为输入模式
 * 输入  ：无
 * 输出  ：无
 */
static void DS18B20_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;

	  	/*选择要控制的DS18B20_DQ_GPIO_PORT引脚*/	
	  GPIO_InitStructure.GPIO_Pin = DS18B20_DQ_GPIO_PIN;

	   /*设置引脚模式为浮空输入模式*/ 
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	

	  /*调用库函数，初始化DS18B20_DQ_GPIO_PORT*/
	  GPIO_Init(DS18B20_DQ_GPIO_PORT, &GPIO_InitStructure);
}


/*
 * 函数名：DS18B20_Mode_Out_PP
 * 描述  ：使DS18B20-DATA引脚变为输出模式
 * 输入  ：无
 * 输出  ：无
 */
static void DS18B20_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

	 	/*选择要控制的DS18B20_DQ_GPIO_PORT引脚*/															   
  	GPIO_InitStructure.GPIO_Pin = DS18B20_DQ_GPIO_PIN;	

	/*设置引脚模式为通用推挽输出*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*设置引脚速率为50MHz */   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/*调用库函数，初始化DS18B20_DQ_GPIO_PORT*/
  	GPIO_Init(DS18B20_DQ_GPIO_PORT, &GPIO_InitStructure);
}


/*
 *主机给从机发送复位脉冲
 */
static void DS18B20_Rst(void)
{
	/* 主机设置为推挽输出 */
	DS18B20_Mode_Out_PP();
	
	DS18B20_DQ_0;
	/* 主机至少产生480us的低电平复位信号 */
	udelay(750);
	
	/* 主机在产生复位信号后，需将总线拉高 */
	DS18B20_DQ_1;
	
	/*从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲*/
	udelay(15);
}


/*
 * 检测从机给主机返回的存在脉冲
 * 0：成功
 * 1：失败
 */
static uint8_t DS18B20_Presence(void)
{
	uint8_t pulse_time = 0;
	
	/* 主机设置为上拉输入 */
	DS18B20_Mode_IPU();
	
	/* 等待存在脉冲的到来，存在脉冲为一个60~240us的低电平信号 
	 * 如果存在脉冲没有来则做超时处理，从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
	 */
	while( DS18B20_DQ_IN() && pulse_time<100 )
	{
		pulse_time++;
		udelay(1);
	}	
	/* 经过100us后，存在脉冲都还没有到来*/
	if( pulse_time >=100 )
		return 1;
	else
		pulse_time = 0;
	
	/* 存在脉冲到来，且存在的时间不能超过240us */
	while( !DS18B20_DQ_IN() && pulse_time<240 )
	{
		pulse_time++;
		udelay(1);
	}	
	if( pulse_time >=240 )
		return 1;
	else
		return 0;
}


/*
 * 从DS18B20读取一个bit
 */
static uint8_t DS18B20_ReadBit(void)
{
	uint8_t dat;
	
	/* 读0和读1的时间至少要大于60us */	
	DS18B20_Mode_Out_PP();
	/* 读时间的起始：必须由主机产生 >1us <15us 的低电平信号 */
	DS18B20_DQ_0;
	udelay(10);
	
	/* 设置成输入，释放总线，由外部上拉电阻将总线拉高 */
	DS18B20_Mode_IPU();
	udelay(2);
	
	if( DS18B20_DQ_IN() == SET )
		dat = 1;
	else
		dat = 0;
	
	/* 这个延时参数请参考时序图 */
	udelay(45);
	
	return dat;
}


/*
 * 从DS18B20读一个字节，低位先行
 */
static uint8_t DS18B20_ReadByte(void)
{
	uint8_t i, j, dat = 0;	
	
	for(i=0; i<8; i++) 
	{
		j = DS18B20_ReadBit();		
		dat = (dat) | (j<<i);
	}
	
	return dat;
}


/*
 * 写一个字节到DS18B20，低位先行
 */
static void DS18B20_WriteByte(uint8_t dat)
{
	uint8_t i, testb;
	DS18B20_Mode_Out_PP();
	
	for( i=0; i<8; i++ )
	{
		testb = dat&0x01;
		dat = dat>>1;		
		/* 写0和写1的时间至少要大于60us */
		if (testb)
		{			
			DS18B20_DQ_0;
			/* 1us < 这个延时 < 15us */
			udelay(8);
			
			DS18B20_DQ_1;
			udelay(58);
		}		
		else
		{			
			DS18B20_DQ_0;
			/* 60us < Tx 0 < 120us */
			udelay(70);
			
			DS18B20_DQ_1;			
			/* 1us < Trec(恢复时间) < 无穷大*/
			udelay(2);
		}
	}
}


 /**
  * @brief  跳过匹配 DS18B20 ROM
  * @param  无
  * @retval 无
  */
static void DS18B20_SkipRom ( void )
{
	DS18B20_Rst();	   
	
	DS18B20_Presence();	 
	
	DS18B20_WriteByte(0XCC);		/* 跳过 ROM */
	
}


 /**
  * @brief  执行匹配 DS18B20 ROM
  * @param  无
  * @retval 无
  */
static void DS18B20_MatchRom ( void )
{
	DS18B20_Rst();	   
	
	DS18B20_Presence();	 
	
	DS18B20_WriteByte(0X55);		/* 匹配 ROM */
	
}


/*
 * 存储的温度是16 位的带符号扩展的二进制补码形式
 * 当工作在12位分辨率时，其中5个符号位，7个整数位，4个小数位
 *
 *         |---------整数----------|-----小数 分辨率 1/(2^4)=0.0625----|
 * 低字节  | 2^3 | 2^2 | 2^1 | 2^0 | 2^(-1) | 2^(-2) | 2^(-3) | 2^(-4) |
 *
 *
 *         |-----符号位：0->正  1->负-------|-----------整数-----------|
 * 高字节  |  s  |  s  |  s  |  s  |    s   |   2^6  |   2^5  |   2^4  |
 *
 * 
 * 温度 = 符号位 + 整数 + 小数*0.0625
 */
 /**
  * @brief  在跳过匹配 ROM 情况下获取 DS18B20 温度值 
  * @param  无
  * @retval 温度值
  */
float DS18B20_GetTemp_SkipRom ( void )
{
	uint8_t tpmsb, tplsb;
	short s_tem;
	float f_tem;
	
	
	DS18B20_SkipRom ();
	DS18B20_WriteByte(0X44);				/* 开始转换 */
	
	
	DS18B20_SkipRom ();
  DS18B20_WriteByte(0XBE);				/* 读温度值 */
	
	tplsb = DS18B20_ReadByte();		 
	tpmsb = DS18B20_ReadByte(); 
	
	
	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;
	
	if( s_tem < 0 )		/* 负温度 */
		f_tem = (~s_tem+1) * 0.0625;	
	else
		f_tem = s_tem * 0.0625;
	
	return f_tem; 	
	
	
}


 /**
  * @brief  在匹配 ROM 情况下获取 DS18B20 温度值 
  * @param  ds18b20_id：用于存放 DS18B20 序列号的数组的首地址
  * @retval 无
  */
void DS18B20_ReadId ( uint8_t * ds18b20_id )
{
	uint8_t uc;
	
	
	DS18B20_WriteByte(0x33);       //读取序列号
	
	for ( uc = 0; uc < 8; uc ++ )
	  ds18b20_id [ uc ] = DS18B20_ReadByte();
	
}

uint32_t DS18B20_GetTemp()
{
	 uint8_t tpmsb, tplsb, i;
	 short s_tem;
	 uint32_t f_tem;
	 
	 DS18B20_MatchRom ();			 //匹配ROM
	 
   for(i=0;i<8;i++)
		 DS18B20_WriteByte ( ds18b20_id [ i ] ); 
	 
	 DS18B20_WriteByte(0X44);				 /* 开始转换 */
 
	 
	 DS18B20_MatchRom ();			 //匹配ROM
	 
	 for(i=0;i<8;i++)
		 DS18B20_WriteByte ( ds18b20_id [ i ] ); 
	 
	 DS18B20_WriteByte(0XBE);				 /* 读温度值 */
	 
	 tplsb = DS18B20_ReadByte();	  
	 tpmsb = DS18B20_ReadByte(); 
	 
	 s_tem = tpmsb<<8;
	 s_tem = s_tem | tplsb;

	 if( s_tem < 0 )	 /* 负温度 */
		 f_tem = (~s_tem+1) * 625;
	 else
		 f_tem = s_tem * 625;
	 
	 return f_tem;	 
	 
	 
 }
 

 /**
  * @brief  在匹配 ROM 情况下获取 DS18B20 温度值 
  * @param  ds18b20_id：存放 DS18B20 序列号的数组的首地址
  * @retval 温度值
  */
float DS18B20_GetTemp_MatchRom ( uint8_t * ds18b20_id )
{
	uint8_t tpmsb, tplsb, i;
	short s_tem;
	float f_tem;
	
	DS18B20_MatchRom ();            //匹配ROM
	
  for(i=0;i<8;i++)
		DS18B20_WriteByte ( ds18b20_id [ i ] );	
	
	DS18B20_WriteByte(0X44);				/* 开始转换 */

	
	DS18B20_MatchRom ();            //匹配ROM
	
	for(i=0;i<8;i++)
		DS18B20_WriteByte ( ds18b20_id [ i ] );	
	
	DS18B20_WriteByte(0XBE);				/* 读温度值 */
	
	tplsb = DS18B20_ReadByte();		 
	tpmsb = DS18B20_ReadByte(); 
	
	
	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;
	
	if( s_tem < 0 )		/* 负温度 */
		f_tem = (~s_tem+1) * 0.0625;	
	else
		f_tem = s_tem * 0.0625;
	
	return f_tem; 	
	
	
}

void ds18b20_main()
{
	static int init = 0;
	
	if (init == 0) {
		DS18B20_Init();
		init = 1;
	}
	
	while (1) {
		//__local_irq_disable(); /* FIXME: udelay depend the irq */
		PRINT_EMG("ds18b20 temp: %d\n",DS18B20_GetTemp());
		//__local_irq_enable();  /* FIXME: udelay depend the irq */
		mdelay(2000);
	}
}
