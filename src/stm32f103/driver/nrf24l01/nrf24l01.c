#include "spi.h"
#include "24l01.h"

u8 TX_ADDRESS[TX_ADR_WIDTH]={0x6d,0x75,0x73,0x65,0x0a};
u8 RX_ADDRESS[RX_ADR_WIDTH]={0x6d,0x75,0x73,0x65,0x0a};

#define TX_CHANNEL_NUM  (100)
#define RX_CHANNEL_NUM  (120)

void nrf24l01_init(void)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure; 
	
 	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4);
	SPI1_Init();
		
	SPI_Cmd(SPI1, DISABLE);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
  
	NRF24L01_CE=0;
	NRF24L01_CSN=1;
}

u8 nrf24l01_check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8);
	nrf24l01_write_buf(NRF_WRITE_REG+TX_ADDR,buf,5);
	nrf24l01_read_buf(TX_ADDR,buf,5);
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;
	return 0;
}	 	 

u8 nrf24l01_write_reg(u8 reg,u8 value)
{
		u8 status;	
   	NRF24L01_CSN=0;
  	status =SPI1_ReadWriteByte(reg);
  	SPI1_ReadWriteByte(value);
  	NRF24L01_CSN=1;
  	return(status);
}

u8 nrf24l01_read_reg(u8 reg)
{
		u8 reg_val;	    
		NRF24L01_CSN = 0;
  	SPI1_ReadWriteByte(reg);
  	reg_val=SPI1_ReadWriteByte(0XFF);
  	NRF24L01_CSN = 1;
  	return(reg_val);
}	

u8 nrf24l01_read_buf(u8 reg,u8 *pBuf,u8 len)
{
		u8 status,u8_ctr;	       
  	NRF24L01_CSN = 0;
  	status=SPI1_ReadWriteByte(reg);
		for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI1_ReadWriteByte(0XFF);
  	NRF24L01_CSN=1;
  	return status;
}

u8 nrf24l01_write_buf(u8 reg, u8 *pBuf, u8 len)
{
		u8 status,u8_ctr;	    
		NRF24L01_CSN = 0;
  	status = SPI1_ReadWriteByte(reg);
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI1_ReadWriteByte(*pBuf++);
  	NRF24L01_CSN = 1;
  	return status;
}				   

u8 nrf24l01_tx(u8 *txbuf)
{
	u8 sta;
 	SPI1_SetSpeed(SPI_BaudRatePrescaler_8);
	NRF24L01_CE=0;
  	nrf24l01_write_buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);
 	NRF24L01_CE=1;
	while(NRF24L01_IRQ!=0);
	sta=nrf24l01_read_reg(STATUS);
	nrf24l01_write_reg(NRF_WRITE_REG+STATUS,sta);
	if(sta&MAX_TX)
	{
		nrf24l01_write_reg(FLUSH_TX,0xff);
		return MAX_TX; 
	}
	if(sta&TX_OK)
	{
		return TX_OK;
	}
	return 0xff;
}

u8 nrf24l01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8);
	sta=nrf24l01_read_reg(STATUS);
	nrf24l01_write_reg(NRF_WRITE_REG+STATUS,sta);
	if(sta&RX_OK)
	{
		nrf24l01_read_buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);
		nrf24l01_write_reg(FLUSH_RX,0xff);
		return 0; 
	}	   
	return 1;
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
	nrf24l01_write_reg(NRF_WRITE_REG+CONFIG,0x12);
	
	nrf24l01_write_reg(NRF_WRITE_REG+EN_AA,0x00); /* no ack */

	nrf24l01_write_reg(NRF_WRITE_REG+EN_RXADDR,0x00); /* no rx */

	nrf24l01_write_reg(NRF_WRITE_REG+SETUP_AW,0x02); /* 4-byte addr */

	nrf24l01_write_reg(NRF_WRITE_REG+SETUP_RETR,0x00); /* no auto-retransmit */

	nrf24l01_write_reg(NRF_WRITE_REG+RF_SETUP,0x06); /* 1Mbps at 0dbm */

	nrf24l01_write_reg(NRF_WRITE_REG+STATUS,0x3e); /* clear flags */

	nrf24l01_write_reg(NRF_WRITE_REG+0x1C,0x00); /* no dynamic payloads */

	nrf24l01_write_reg(NRF_WRITE_REG+0x1D,0x00); /* no features */

	nrf24l01_write_reg(NRF_WRITE_REG+0x1D,0x00); /* no features */

	nrf24l01_write_reg(NRF_WRITE_REG+0x11,32); /* always rx 32 bytes */

	nrf24l01_write_reg(NRF_WRITE_REG+EN_RXADDR, 0x01); /* rx on pipe 0 */

  TX_ADDRESS[0] = swapbits(0x8E);
  TX_ADDRESS[1] = swapbits(0x89);
  TX_ADDRESS[2] = swapbits(0xBE);
  TX_ADDRESS[3] = swapbits(0xD6);
	
	nrf24l01_write_buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)TX_ADDRESS, 4);//写TX节点地址 

	nrf24l01_write_buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)TX_ADDRESS, 4);//写TX节点地址 


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
			  buf[L++] = 0x11;
			  buf[L++] = 0x22;	
			  buf[L++] = 0x33;// some test data
				  
			  buf[L++] = 0x55;	//CRC start value: 0x555555
			  buf[L++] = 0x55;
			  buf[L++] = 0x55;
		
			  nrf24l01_write_reg(NRF_WRITE_REG+RF_CH, chRf[ch]);
			  
			  nrf24l01_write_reg(NRF_WRITE_REG+STATUS, 0x6E); // clear flags
			  	
				  
			  btLePacketEncode(buf, L, chLe[ch]);
			  
			  nrf24l01_write_reg(FLUSH_TX,0xff); //Clear TX Fifo
			  nrf24l01_write_reg(FLUSH_RX,0xff); //Clear RX Fifo		

			  nrf24l01_tx(buf);
			  mdelay(2);
		  }

		  mdelay(200);	 // Broadcasting interval

	}
}

