#if 1

//#include "iic.h"
#include "stm32f10x.h"

#include "timer.h"
#include "systick.h"       //调用了延时函数
#include "log.h"

//#include <math.h>
 
#define I2C_SCL_PIN		GPIO_Pin_7
#define I2C_SDA_PIN		GPIO_Pin_5

//SCL -> PC1
//SDA -> PC2
#define SCL_H()  GPIO_SetBits(GPIOA, I2C_SCL_PIN)
#define SCL_L()  GPIO_ResetBits(GPIOA, I2C_SCL_PIN)
#define SDA_H()  GPIO_SetBits(GPIOA, I2C_SDA_PIN)
#define SDA_L()  GPIO_ResetBits(GPIOA, I2C_SDA_PIN)
 
#define SDA  GPIO_ReadInputDataBit(GPIOA,I2C_SDA_PIN)

#define SDA_INPUT()		IIC_SDAIntput()
#define SDA_OUTPUT()	IIC_SDAOutput()


//#define GT911_SlaveAddr 0x14

#define GT911_SlaveAddr 0x28

#define DELAY (100)

void IIC_SDAOutput()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_Out_OD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

void IIC_SDAIntput()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

static void IIC_Init(void)
{
		SDA_OUTPUT();
	
    SCL_H();  //SCL = 1;
    udelay(DELAY);
    SDA_H();  //SDA = 1;
    udelay(DELAY);    
}

static void IIC_Start(void)
{
		SDA_OUTPUT();
	
    SDA_H();  //SDA = 1;
    udelay(DELAY);
    SCL_H();  //SCL = 1;
    udelay(DELAY);
    SDA_L();  //SDA = 0;
    udelay(DELAY);    
}
 
static void IIC_Stop(void)
{
		SDA_OUTPUT();
	
    SDA_L();   //SDA = 0;
    udelay(DELAY);
    SCL_H();   //SCL = 1;
    udelay(DELAY);
    SDA_H();   //SDA = 1;
    udelay(DELAY);
}
 
static unsigned char IIC_ReceiveACK(void)
{
    unsigned char ACK;
		
		SDA_INPUT();
	
    SDA_H();     //SDA=1;//要读低电平需先拉高再读,否则读到的是错误数据,很重要！
    SCL_H();     //SCL=1;
    udelay(DELAY);
 
    if (SDA==1)  //SDA为高
    {
        ACK = 1;    
    }
    else ACK = 0;  //SDA为低
 
 
    SCL_L();    //SCL = 0;//SCL为低电平时SDA上的数据才允许变化,为传送下一个字节做准备 
    udelay(DELAY);
     
    return ACK;                 
}

/* ack: 0 ACK; 1 NACK */
static void IIC_SendACK(unsigned char ack)
{
		SDA_OUTPUT();

		SCL_L();
	
    if (ack == 1)SDA_H();	/* NACK */
    else if (ack == 0)SDA_L(); /* ACK */
    //SDA = ack;
    SCL_H();   //SCL = 1;
    udelay(DELAY);
    SCL_L();   //SCL = 0;
    udelay(DELAY);
	
}
 
static unsigned char IIC_SendByte(unsigned char dat)
{
    unsigned char i;
    unsigned char bResult=1;

		SDA_OUTPUT();

    SCL_L();     //SCL = 0;//拉低时钟线
    udelay(DELAY);        
 
    for(i = 0; i < 8; i++) //一个SCK,把dat一位一位的移送到SDA上
    {
        if( (dat<<i)&0x80 ) 
					SDA_H();   //SDA = 1;//先发高位
        else 
					SDA_L();  //SDA = 0;
        udelay(DELAY);
 
        SCL_H();  //SCL = 1;
        udelay(DELAY);
        SCL_L();  //SCL = 0;
        udelay(DELAY);
    }
 
  bResult = IIC_ReceiveACK(); //发送完一个字节的数据,等待接受应答信号
 	if (bResult) {
		PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
	}
    return bResult;  //返回应答信号
}

static unsigned char IIC_SendByte_Nak(unsigned char dat)
{
    unsigned char i;
    unsigned char bResult=1;
     
    SCL_L();     //SCL = 0;//拉低时钟线
    udelay(DELAY);        
 
    for(i = 0; i < 8; i++) //一个SCK,把dat一位一位的移送到SDA上
    {
        if( (dat<<i)&0x80 )
					SDA_H();   //SDA = 1;//先发高位
        else 
					SDA_L();  //SDA = 0;
        udelay(DELAY);
 
        SCL_H();  //SCL = 1;
        udelay(DELAY);
        SCL_L();  //SCL = 0;
        udelay(DELAY);
    }
 
	IIC_SendACK(1);
#if 0
	SCL_L();
	SDA_H();
	udelay(DELAY);
	SCL_H();
	udelay(DELAY);
	/* check the SDA */
	SCL_L();
#endif	
	//SDA_L();
	//SCL_L();
    return 0;  //返回应答信号
}

static unsigned char IIC_ReadByte(void)
{
    unsigned char dat = 0;
    unsigned char i;

		SDA_INPUT();
	
    SCL_H();     //SCL = 1;//始终线拉高为读数据做准备
	
    udelay(DELAY);
 
    for( i=0;i<8;i++ )
    {
        dat <<= 1;
        dat = dat | (SDA);
        udelay(DELAY);
         
        SCL_L();   //SCL = 0;
        udelay(DELAY);    
        SCL_H();   //SCL = 1;
        udelay(DELAY);
			
    }
    return dat;
}

/*外部芯片IIC引脚初始化
 *SCL:PC1
 *SDA:PC2
*/
static void IIC_PortInit(void)
{
#if 0
    GPIO_InitTypeDef GPIO_InitStructure;  //定义一个GPIO_InitTypeDef类型的结构体
 
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_1|GPIO_Pin_2);    //PC1,PC2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;           //漏极开漏
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
 
    GPIO_SetBits(GPIOC, GPIO_Pin_1|GPIO_Pin_2);    //拉高
#else
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; /* GPIO_Mode_Out_OD; */	/* 开漏输出 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
	IIC_Stop();

#endif
}

void gt911_write_reg(uint16_t addr, uint32_t size, uint8_t *buf)
{
	uint32_t i;
	
	PRINT_EMG("write 0x%x\n", addr);
	
	/* 1. START */
	IIC_Start();
	
	/* 2. send slave addr with Write */
	IIC_SendByte(GT911_SlaveAddr | 0x0); /* 0x0: W; 0x1: R */
	
	/* 3. send reg addr */
	IIC_SendByte(addr >> 8);
	
	IIC_SendByte(addr & 0xFF);

	/* 4. send data */
	for(i = 0; i < size; i++) {
		IIC_SendByte(buf[i]);
	}
	
	/* 5. STOP */
	IIC_Stop();

}

void gt911_read_reg(uint16_t addr, uint32_t size, uint8_t *buf)
{
	uint32_t i;
	
	PRINT_EMG("read 0x%x\n", addr);

	/* 1. START */
	IIC_Start();
	
	/* 2. send slave addr with Write */
	IIC_SendByte(GT911_SlaveAddr | 0x0); /* 0x0: W; 0x1: R */
	
	/* 3. send reg addr */
	IIC_SendByte(addr >> 8);
	
	IIC_SendByte(addr & 0xFF);

	/* 4. START */
	IIC_Start();
	
	/* 5. send slave addr with Read */
	IIC_SendByte(GT911_SlaveAddr | 0x1); /* 0x0: W; 0x1: R */
	
	for(i = 0; i < size; i++)	{
		buf[i] = IIC_ReadByte();

		if (i == (size - 1)) {
			IIC_SendACK(1); /* NACK */
		} else {
			IIC_SendACK(0);
		}
		
		PRINT_EMG("[0x%x]: 0x%x\n", addr + i, buf[i]);
		udelay(10000);
	}

	IIC_Stop();	
}

void gt911_main()
{
	int i, rv;
	char buf[16] = {0};
	
	PRINT_EMG("%s-%d\n", __func__, __LINE__);
	IIC_PortInit();

#if 1
	for (i = 0; i < 256; i++) {
		if (i == 0x28 || i == 0x29 || i == 0x2A ||
			  i == 0xBA || i == 0xBB || i == 0xBC) {				
			PRINT_EMG("addr [0x%X]: ", i);
			IIC_Start();     //IIC start
			rv = IIC_SendByte(i);   //slave address+W:0	
			if (rv) {
				PRINT_EMG("fail!\n");
			} else {
				PRINT_EMG("succ!\n");
			}
			
			IIC_Stop();
		}
	}
#endif
	
	//IIC_Start();
  //IIC_SendByte(GT911_SlaveAddr);

	gt911_read_reg(0x8140, 2, buf);

	for(i = 0; i < 16; i++) {
		PRINT_EMG("buf[%d]: 0x%x\n", i, buf[i]);
	}
	
	gt911_read_reg(0x8141, 2, buf);
	gt911_read_reg(0x8142, 2, buf);
	gt911_read_reg(0x8143, 2, buf);

	//IIC_Stop();


}

#endif













































































/****************************************************************************************************************************************/




#if 0

#include "stm32f10x.h"

#include "timer.h"
#include "systick.h"       //调用了延时函数
#include "log.h"

#define I2C_SCL_PIN		GPIO_Pin_7
#define I2C_SDA_PIN		GPIO_Pin_5

#define CT_SCL_H()  GPIO_SetBits(GPIOA, I2C_SCL_PIN)
#define CT_SCL_L()  GPIO_ResetBits(GPIOA, I2C_SCL_PIN)
#define CT_SDA_H()  GPIO_SetBits(GPIOA, I2C_SDA_PIN)
#define CT_SDA_L()  GPIO_ResetBits(GPIOA, I2C_SDA_PIN)

#define CT_SDA_OUT() IIC_SDAOutput()
#define CT_SDA_IN()  IIC_SDAIutput()

#define CT_READ_SDA  GPIO_ReadInputDataBit(GPIOA,I2C_SDA_PIN)

void IIC_SDAOutput()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_IN_FLOATING;	/* 开漏输出 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

void IIC_SDAIutput()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	/* 开漏输出 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

static void IIC_PortInit(void)
{
#if 0
    GPIO_InitTypeDef GPIO_InitStructure;  //定义一个GPIO_InitTypeDef类型的结构体
 
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_1|GPIO_Pin_2);    //PC1,PC2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;           //漏极开漏
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
 
    GPIO_SetBits(GPIOC, GPIO_Pin_1|GPIO_Pin_2);    //拉高
#else
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	/* 开漏输出 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
	//IIC_Stop();

#endif
}


//控制I2C速度的延时
void CT_Delay(void)
{
	udelay(20);
}

//电容触摸芯片IIC接口初始化
void CT_IIC_Init(void)
{					     
	IIC_PortInit();
	
	CT_SDA_H();  	  
	CT_SCL_H();
}
//产生IIC起始信号
void CT_IIC_Start(void)
{
	CT_SDA_OUT();     //sda线输出
	CT_SDA_H();	  	  
	CT_SCL_H();
	udelay(30);
 	CT_SDA_L();//START:when CLK is high,DATA change form high to low 
	CT_Delay();
	CT_SCL_L();//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void CT_IIC_Stop(void)
{
	CT_SDA_OUT();//sda线输出
	CT_SCL_L(); 
	CT_SDA_L();//STOP:when CLK is high DATA change form low to high
	udelay(30);
	CT_SCL_H(); 
	CT_Delay(); 
	CT_SDA_H();//发送I2C总线结束信号  
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 CT_IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	CT_SDA_IN();      //SDA设置为输入  
	CT_SDA_H();	   
	CT_SCL_H();
	CT_Delay();
	while(CT_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			CT_IIC_Stop();
			return 1;
		} 
		CT_Delay();
	}
	CT_SCL_L();//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void CT_IIC_Ack(void)
{
	CT_SCL_L();
	CT_SDA_OUT(); 
	CT_SDA_L();
	CT_Delay();
	CT_SCL_H();
	CT_Delay();
	CT_SCL_L();
}
//不产生ACK应答		    
void CT_IIC_NAck(void)
{
	CT_SCL_L();
	CT_SDA_OUT(); 
	CT_SDA_H();
	CT_Delay();
	CT_SCL_H();
	CT_Delay();
	CT_SCL_L();
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void CT_IIC_Send_Byte(u8 txd)
{                        
	u8 t;   
	CT_SDA_OUT(); 	    
	CT_SCL_L();//拉低时钟开始数据传输 
	for(t=0;t<8;t++) {              
		//CT_IIC_SDA = (txd&0x80)>>7;

		if ((txd&0x80)>>7) {
			CT_SDA_H();
		} else {
			CT_SDA_L();
		}
		
		txd<<=1; 
		CT_Delay();	      
		CT_SCL_H(); 
		CT_Delay();
		CT_SCL_L();	 
		CT_Delay();
	}	 
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 CT_IIC_Read_Byte(unsigned char ack)
{
	u8 i,receive=0;
 	CT_SDA_IN();//SDA设置为输入 
	udelay(30);
	for(i=0;i<8;i++ )
	{ 
		CT_SCL_L(); 	    	   
		CT_Delay();
		CT_SCL_H();	 
		receive<<=1;
		if(CT_READ_SDA)receive++; 
		CT_Delay();  
	}	  				 
	if (!ack)CT_IIC_NAck();//发送nACK
	else CT_IIC_Ack(); //发送ACK   
 	return receive;
}


//#define GT_CMD_WR 		0X28
//#define GT_CMD_RD 		0X29

#define GT_CMD_WR 		0XBA
#define GT_CMD_RD 		0XBB


#define GT_CTRL_REG 	0X8040
#define GT_CFGS_REG 	0X8047
#define GT_CHECK_REG 	0X80FF
#define GT_PID_REG 		0X8140

#define GT_GSTID_REG 	0X814E
#define GT_TP1_REG 		0X8150
#define GT_TP2_REG 		0X8158
#define GT_TP3_REG 		0X8160
#define GT_TP4_REG 		0X8168
#define GT_TP5_REG 		0X8170


void GT911_RD_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   //????? 	 
	CT_IIC_Wait_Ack();
 	CT_IIC_Send_Byte(reg>>8);   	//???8???
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//???8???  
	CT_IIC_Wait_Ack();  
	 
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(GT_CMD_RD);   //?????		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //???	  
	} 
    CT_IIC_Stop();//????????    
} 

void gt911_main()
{
	uint8_t i, rv;
	uint8_t temp[4];
	
#if 1
	for (i = 0; i < 255; i++) {

		PRINT_EMG("addr [0x%X]: ", i);
		
	  CT_IIC_Start();     //IIC start
		
    CT_IIC_Send_Byte(i);   //slave address+W:0	
		rv = CT_IIC_Wait_Ack();
		if (rv) {
			PRINT_EMG("fail!\n");
		} else {
			PRINT_EMG("succ!\n");
		}
		
		CT_IIC_Stop();
	}
#endif
	
	CT_IIC_Init();
	
	GT911_RD_Reg(GT_PID_REG,temp,4);//????ID
	
	//temp[4]=0;
	PRINT_EMG("%s\r\n",temp);	//??ID
	PRINT_EMG("TouchPad_ID:%d,%d,%d\r\n",temp[0],temp[1],temp[2]);
	
}

#endif
