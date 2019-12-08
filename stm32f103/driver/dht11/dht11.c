#include <stm32f10x.h>
#include "log.h"

static void DHT11_GPIO_Config ( void );

typedef struct
{
	uint8_t  humi_int;		//Êª¶ÈµÄÕûÊı²¿·Ö
	uint8_t  humi_deci;	 	//Êª¶ÈµÄĞ¡Êı²¿·Ö
	uint8_t  temp_int;	 	//ÎÂ¶ÈµÄÕûÊı²¿·Ö
	uint8_t  temp_deci;	 	//ÎÂ¶ÈµÄĞ¡Êı²¿·Ö
	uint8_t  check_sum;	 	//Ğ£ÑéºÍ
		                 
} DHT11_Data_TypeDef;


/************************** DHT11 é‘º??æŒå©…æ™¯y???éˆ¥æ»†æ™¯?********************************/
#define      DHT11_Dout_SCK_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define      DHT11_Dout_GPIO_CLK                       RCC_APB2Periph_GPIOA

#define      DHT11_Dout_GPIO_PORT                      GPIOA
#define      DHT11_Dout_GPIO_PIN                       GPIO_Pin_6

#define      DHT11_Dout_0	                            GPIO_ResetBits ( DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN ) 
#define      DHT11_Dout_1	                            GPIO_SetBits ( DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN ) 

#define      DHT11_Dout_IN()	                          GPIO_ReadInputDataBit ( DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN ) 



 /**
  * @brief  DHT11 ³õÊ¼»¯º¯Êı
  * @param  ÎŞ
  * @retval ÎŞ
  */
void DHT11_Init ( void )
{
	DHT11_GPIO_Config ();
	
	DHT11_Dout_1;               // À­¸ßGPIOB10
}


/*
 * º¯ÊıÃû£ºDHT11_GPIO_Config
 * ÃèÊö  £ºÅäÖÃDHT11ÓÃµ½µÄI/O¿Ú
 * ÊäÈë  £ºÎŞ
 * Êä³ö  £ºÎŞ
 */
static void DHT11_GPIO_Config ( void )
{		
	/*¶¨ÒåÒ»¸öGPIO_InitTypeDefÀàĞÍµÄ½á¹¹Ìå*/
	GPIO_InitTypeDef GPIO_InitStructure; 

	
	/*¿ªÆôDHT11_Dout_GPIO_PORTµÄÍâÉèÊ±ÖÓ*/
  DHT11_Dout_SCK_APBxClock_FUN ( DHT11_Dout_GPIO_CLK, ENABLE );	
 
	/*Ñ¡ÔñÒª¿ØÖÆµÄDHT11_Dout_GPIO_PORTÒı½Å*/															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;	

	/*ÉèÖÃÒı½ÅÄ£Ê½ÎªÍ¨ÓÃÍÆÍìÊä³ö*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*ÉèÖÃÒı½ÅËÙÂÊÎª50MHz */   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*µ÷ÓÃ¿âº¯Êı£¬³õÊ¼»¯DHT11_Dout_GPIO_PORT*/
  	GPIO_Init ( DHT11_Dout_GPIO_PORT, &GPIO_InitStructure );		  
	
}


/*
 * º¯ÊıÃû£ºDHT11_Mode_IPU
 * ÃèÊö  £ºÊ¹DHT11-DATAÒı½Å±äÎªÉÏÀ­ÊäÈëÄ£Ê½
 * ÊäÈë  £ºÎŞ
 * Êä³ö  £ºÎŞ
 */
static void DHT11_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;

	  	/*Ñ¡ÔñÒª¿ØÖÆµÄDHT11_Dout_GPIO_PORTÒı½Å*/	
	  GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;

	   /*ÉèÖÃÒı½ÅÄ£Ê½Îª¸¡¿ÕÊäÈëÄ£Ê½*/ 
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 

	  /*µ÷ÓÃ¿âº¯Êı£¬³õÊ¼»¯DHT11_Dout_GPIO_PORT*/
	  GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure);	 
	
}


/*
 * º¯ÊıÃû£ºDHT11_Mode_Out_PP
 * ÃèÊö  £ºÊ¹DHT11-DATAÒı½Å±äÎªÍÆÍìÊä³öÄ£Ê½
 * ÊäÈë  £ºÎŞ
 * Êä³ö  £ºÎŞ
 */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

	 	/*Ñ¡ÔñÒª¿ØÖÆµÄDHT11_Dout_GPIO_PORTÒı½Å*/															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;	

	/*ÉèÖÃÒı½ÅÄ£Ê½ÎªÍ¨ÓÃÍÆÍìÊä³ö*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*ÉèÖÃÒı½ÅËÙÂÊÎª50MHz */   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/*µ÷ÓÃ¿âº¯Êı£¬³õÊ¼»¯DHT11_Dout_GPIO_PORT*/
  	GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure);	 	 
	
}


/* 
 * ´ÓDHT11¶ÁÈ¡Ò»¸ö×Ö½Ú£¬MSBÏÈĞĞ
 */
static uint8_t DHT11_ReadByte ( void )
{
	uint8_t i, temp=0;
	

	for(i=0;i<8;i++)    
	{	 
		/*Ã¿bitÒÔ50usµÍµçÆ½±êÖÃ¿ªÊ¼£¬ÂÖÑ¯Ö±µ½´Ó»ú·¢³ö µÄ50us µÍµçÆ½ ½áÊø*/  
		while(DHT11_Dout_IN()==Bit_RESET);

		/*DHT11 ÒÔ26~28usµÄ¸ßµçÆ½±íÊ¾¡°0¡±£¬ÒÔ70us¸ßµçÆ½±íÊ¾¡°1¡±£¬
		 *Í¨¹ı¼ì²â x usºóµÄµçÆ½¼´¿ÉÇø±ğÕâÁ½¸ö×´ £¬x ¼´ÏÂÃæµÄÑÓÊ± 
		 */
		udelay(40); //ÑÓÊ±x us Õâ¸öÑÓÊ±ĞèÒª´óÓÚÊı¾İ0³ÖĞøµÄÊ±¼ä¼´¿É	   	  

		if(DHT11_Dout_IN()==Bit_SET)/* x usºóÈÔÎª¸ßµçÆ½±íÊ¾Êı¾İ¡°1¡± */
		{
			/* µÈ´ıÊı¾İ1µÄ¸ßµçÆ½½áÊø */
			while(DHT11_Dout_IN()==Bit_SET);

			temp|=(uint8_t)(0x01<<(7-i));  //°ÑµÚ7-iÎ»ÖÃ1£¬MSBÏÈĞĞ 
		}
		else	 // x usºóÎªµÍµçÆ½±íÊ¾Êı¾İ¡°0¡±
		{			   
			temp&=(uint8_t)~(0x01<<(7-i)); //°ÑµÚ7-iÎ»ÖÃ0£¬MSBÏÈĞĞ
		}
	}
	
	return temp;
	
}


/*
 * Ò»´ÎÍêÕûµÄÊı¾İ´«ÊäÎª40bit£¬¸ßÎ»ÏÈ³ö
 * 8bit Êª¶ÈÕûÊı + 8bit Êª¶ÈĞ¡Êı + 8bit ÎÂ¶ÈÕûÊı + 8bit ÎÂ¶ÈĞ¡Êı + 8bit Ğ£ÑéºÍ 
 */
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{  
	/*Êä³öÄ£Ê½*/
	DHT11_Mode_Out_PP();
	/*Ö÷»úÀ­µÍ*/
	DHT11_Dout_0;
	/*ÑÓÊ±18ms*/
	mdelay(18);

	/*×ÜÏßÀ­¸ß Ö÷»úÑÓÊ±30us*/
	DHT11_Dout_1; 

	udelay(30);   //ÑÓÊ±30us

	/*Ö÷»úÉèÎªÊäÈë ÅĞ¶Ï´Ó»úÏìÓ¦ĞÅºÅ*/ 
	DHT11_Mode_IPU();

	/*ÅĞ¶Ï´Ó»úÊÇ·ñÓĞµÍµçÆ½ÏìÓ¦ĞÅºÅ Èç²»ÏìÓ¦ÔòÌø³ö£¬ÏìÓ¦ÔòÏòÏÂÔËĞĞ*/   
	if(DHT11_Dout_IN()==Bit_RESET)     
	{
		/*ÂÖÑ¯Ö±µ½´Ó»ú·¢³ö µÄ80us µÍµçÆ½ ÏìÓ¦ĞÅºÅ½áÊø*/  
		while(DHT11_Dout_IN()==Bit_RESET);

		/*ÂÖÑ¯Ö±µ½´Ó»ú·¢³öµÄ 80us ¸ßµçÆ½ ±êÖÃĞÅºÅ½áÊø*/
		while(DHT11_Dout_IN()==Bit_SET);

		/*¿ªÊ¼½ÓÊÕÊı¾İ*/   
		DHT11_Data->humi_int= DHT11_ReadByte();

		DHT11_Data->humi_deci= DHT11_ReadByte();

		DHT11_Data->temp_int= DHT11_ReadByte();

		DHT11_Data->temp_deci= DHT11_ReadByte();

		DHT11_Data->check_sum= DHT11_ReadByte();


		/*¶ÁÈ¡½áÊø£¬Òı½Å¸ÄÎªÊä³öÄ£Ê½*/
		DHT11_Mode_Out_PP();
		/*Ö÷»úÀ­¸ß*/
		DHT11_Dout_1;

		/*¼ì²é¶ÁÈ¡µÄÊı¾İÊÇ·ñÕıÈ·*/
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
			return SUCCESS;
		else 
			return ERROR;
	}
	
	else
		return ERROR;
	
}

void dht11_main()
{
	static int init = 0;
	DHT11_Data_TypeDef DHT11_Data;
		
	if (init == 0) {
		DHT11_Init();
		init = 1;
	}

	while (1) {
		if( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
		{	
			PRINT_EMG("dht11: humidity: %d.%d; temperature %d.%d \n",
				DHT11_Data.humi_int,DHT11_Data.humi_deci,
				DHT11_Data.temp_int,DHT11_Data.temp_deci);
		}			
		else
		{
			PRINT_EMG("Read DHT11 ERROR!\r\n");
		}
		
		mdelay(1000);		
	}
}

