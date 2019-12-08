#include "spi.h"
#include "24l01.h"

u8 TX_ADDRESS[TX_ADR_WIDTH]={0x6d,0x75,0x73,0x65,0x0a}; //发送地址
u8 RX_ADDRESS[RX_ADR_WIDTH]={0x6d,0x75,0x73,0x65,0x0a}; //发送地址

#define TX_CHANNEL_NUM  (100)
#define RX_CHANNEL_NUM  (120)


//初始化24L01的IO口
void NRF24L01_Init(void)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure; 
	
 	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#if 0	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	GPIO_SetBits(GPIOC,GPIO_Pin_4);
#endif
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU  ;   //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4);
	SPI1_Init();    		//初始化SPI
		
	SPI_Cmd(SPI1, DISABLE); // 
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//选择了串行时钟的稳态:时钟悬空低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第一个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	//NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x02); /* power up */
    //delay_ms(2);
    
	NRF24L01_CE=0; 	//使能24L01
	NRF24L01_CSN=1;	//SPI片选取消	 	
}
//检测24L01是否存在
//返回值:0，成功;1，失败	
u8 NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   	 
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
}	 	 
//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
   	NRF24L01_CSN=0;                 //使能SPI传输
  	status =SPI1_ReadWriteByte(reg);//发送寄存器号 
  	SPI1_ReadWriteByte(value);      //写入寄存器的值
  	NRF24L01_CSN=1;                 //禁止SPI传输	   
  	return(status);       			//返回状态值
}
//读取SPI寄存器值
//reg:要读的寄存器
u8 NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;	    
 	NRF24L01_CSN = 0;          //使能SPI传输		
  	SPI1_ReadWriteByte(reg);   //发送寄存器号
  	reg_val=SPI1_ReadWriteByte(0XFF);//读取寄存器内容
  	NRF24L01_CSN = 1;          //禁止SPI传输		    
  	return(reg_val);           //返回状态值
}	
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;	       
  	NRF24L01_CSN = 0;           //使能SPI传输
  	status=SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI1_ReadWriteByte(0XFF);//读出数据
  	NRF24L01_CSN=1;       //关闭SPI传输
  	return status;        //返回读到的状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
 	NRF24L01_CSN = 0;          //使能SPI传输
  	status = SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI1_ReadWriteByte(*pBuf++); //写入数据	 
  	NRF24L01_CSN = 1;       //关闭SPI传输
  	return status;          //返回读到的状态值
}				   
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
u8 NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;
 	SPI1_SetSpeed(SPI_BaudRatePrescaler_8);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	NRF24L01_CE=0;
  	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF24L01_CE=1;//启动发送	   
	while(NRF24L01_IRQ!=0);//等待发送完成
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)//达到最大重发次数
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);//清除TX FIFO寄存器 
		return MAX_TX; 
	}
	if(sta&TX_OK)//发送完成
	{
		return TX_OK;
	}
	return 0xff;//其他原因发送失败
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(FLUSH_RX,0xff);//清除RX FIFO寄存器 
		return 0; 
	}	   
	return 1;//没收到任何数据
}					    
//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void NRF24L01_RX_Mode(void)
{
	NRF24L01_CE=0;	  
  	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
	  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);    	//使能通道0的自动应答    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);	//使能通道0的接收地址  	 
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,RX_CHANNEL_NUM);	    	//设置RF通信频率		  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);	//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f);		//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  	NRF24L01_CE = 1; //CE为高,进入接收模式 
  	udelay(150);
}

//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了		   
//CE为高大于10us,则启动发送.	 
void NRF24L01_TX_Mode(void)
{														 
	NRF24L01_CE=0;	    
  	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址 
  	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  

  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     //使能通道0的自动应答    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x1f);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,TX_CHANNEL_NUM);       //设置RF通道为40
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	NRF24L01_CE=1;//CE为高,10us后启动发送
	udelay(150);
}



/*************** BLE ***************/

static uint8_t buf[32];   
static const uint8_t chRf[] = {2, 26,80};
static const uint8_t chLe[] = {37,38,39};
static uint8_t ch = 0;  // RF channel for frequency hopping

void btLeCrc(const uint8_t* data, uint8_t len, uint8_t* dst){
// implementing CRC with LFSR
    uint8_t v, t, d;
    
    while(len--){ 
        d = *data++;
        for(v = 0; v < 8; v++, d >>= 1){
            t = dst[0] >> 7;
            dst[0] <<= 1;
            if(dst[1] & 0x80) dst[0] |= 1;
            dst[1] <<= 1;
            if(dst[2] & 0x80) dst[1] |= 1;
            dst[2] <<= 1;
            
            if(t != (d & 1)){
              dst[2] ^= 0x5B;
              dst[1] ^= 0x06;
            }
        } 
    }
}

uint8_t  swapbits(uint8_t a){
  // reverse the bit order in a single byte
    uint8_t v = 0;
    if(a & 0x80) v |= 0x01;
    if(a & 0x40) v |= 0x02;
    if(a & 0x20) v |= 0x04;
    if(a & 0x10) v |= 0x08;
    if(a & 0x08) v |= 0x10;
    if(a & 0x04) v |= 0x20;
    if(a & 0x02) v |= 0x40;
    if(a & 0x01) v |= 0x80;
    return v;
}

void btLeWhiten(uint8_t* data, uint8_t len, uint8_t whitenCoeff){
// Implementing whitening with LFSR
    uint8_t  m;
    while(len--){
        for(m = 1; m; m <<= 1){
            if(whitenCoeff & 0x80){
                whitenCoeff ^= 0x11;
                (*data) ^= m;
            }
            whitenCoeff <<= 1;
        }
        data++;
    }
}

static uint8_t btLeWhitenStart(uint8_t chan){
//the value we actually use is what BT'd use left shifted one...makes our life easier
    return swapbits(chan) | 2;  
}

void btLePacketEncode(uint8_t* packet, uint8_t len, uint8_t chan){
// Assemble the packet to be transmitted
// Length is of packet, including crc. pre-populate crc in packet with initial crc value!
    uint8_t i, dataLen = len - 3;
    btLeCrc(packet, dataLen, packet + dataLen);
    for(i = 0; i < 3; i++, dataLen++) 
        packet[dataLen] = swapbits(packet[dataLen]);
    btLeWhiten(packet, len, btLeWhitenStart(chan));
    for(i = 0; i < len; i++) 
        packet[i] = swapbits(packet[i]); // the byte order of the packet should be reversed as well
  
}

#define MY_MAC_0  0x11
#define MY_MAC_1  0x22
#define MY_MAC_2  0x33
#define MY_MAC_3  0x44
#define MY_MAC_4  0x55
#define MY_MAC_5  0x66


void nrf24l01_ble_broadcast()
{
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x12);
	
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x00); /* no ack */

	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x00); /* no rx */

	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_AW,0x02); /* 4-byte addr */

	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x00); /* no auto-retransmit */

	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x06); /* 1Mbps at 0dbm */

	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,0x3e); /* clear flags */

	NRF24L01_Write_Reg(NRF_WRITE_REG+0x1C,0x00); /* no dynamic payloads */

	NRF24L01_Write_Reg(NRF_WRITE_REG+0x1D,0x00); /* no features */

	NRF24L01_Write_Reg(NRF_WRITE_REG+0x1D,0x00); /* no features */

	NRF24L01_Write_Reg(NRF_WRITE_REG+0x11,32); /* always rx 32 bytes */

	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR, 0x01); /* rx on pipe 0 */

	TX_ADDRESS[0] = swapbits(0x8E);
    TX_ADDRESS[1] = swapbits(0x89);
    TX_ADDRESS[2] = swapbits(0xBE);
    TX_ADDRESS[3] = swapbits(0xD6);
	
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)TX_ADDRESS, 4);//写TX节点地址 

	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)TX_ADDRESS, 4);//写TX节点地址 


	while (1) {
		for (ch=0; ch<sizeof(chRf); ch++)
		  { 
			  uint8_t i, L=0;
			  
			  buf[L++] = 0x42;	//PDU type, given address is random; 0x42 for Android and 0x40 for iPhone
			  buf[L++] = 16+5; // length of payload
				  
			  buf[L++] = MY_MAC_0;
			  buf[L++] = MY_MAC_1;
			  buf[L++] = MY_MAC_2;
			  buf[L++] = MY_MAC_3;
			  buf[L++] = MY_MAC_4;
			  buf[L++] = MY_MAC_5;
				  
			  buf[L++] = 2;   //flags (LE-only, general discoverable mode)
			  buf[L++] = 0x01;
			  buf[L++] = 0x06;
			  
			  buf[L++] = 6;   // length of the name, including type byte
			  buf[L++] = 0x08;
			  buf[L++] = 'n';
			  buf[L++] = 'R';
			  buf[L++] = 'F';
			  buf[L++] = '2';
			  buf[L++] = '4';
				  
			  buf[L++] = 4;   // length of custom data, including type byte
			  buf[L++] = 0xff;	 
			  buf[L++] = 0x01;
			  buf[L++] = 0x02;	
			  buf[L++] = 0x03;// some test data
				  
			  buf[L++] = 0x55;	//CRC start value: 0x555555
			  buf[L++] = 0x55;
			  buf[L++] = 0x55;
		
			  NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH, chRf[ch]);
			  
			  NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS, 0x6E); // clear flags
			  	
				  
			  btLePacketEncode(buf, L, chLe[ch]);
			  
			  NRF24L01_Write_Reg(FLUSH_TX,0xff); //Clear TX Fifo
			  NRF24L01_Write_Reg(FLUSH_RX,0xff); //Clear RX Fifo		

			  NRF24L01_TxPacket(buf);
			  mdelay(2);
		  }
		
		  mdelay(200);	 // Broadcasting interval

	}
}




