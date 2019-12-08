#include "timer.h"

#include "log.h"

extern uint8_t ee_ReadByte(uint8_t device_addr, uint16_t _usAddress);

#if 0
#define TSL2561_ADDR	(0x39)

#define BMP180_ADDR		(0x77)

#define AM2321_ADDR		(0x5C)
#endif
#define TSL2561_ADDR	(0x39)

#define BMP180_ADDR		(0xEE)

#define AM2321_ADDR		(0x5C)


void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);


uint8_t i2c_checkdevice(uint8_t _Address)
{
	uint8_t ucAck;
	void i2c_CfgGpio(void);

	i2c_CfgGpio();		/* 配置GPIO */

	
	i2c_Start();		/* 发送启动信号 */

	/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
	i2c_SendByte(_Address);
	ucAck = i2c_WaitAck();	/* 检测设备的ACK应答 */

	i2c_Stop();			/* 发送停止信号 */

	return ucAck;
}

uint8_t i2c_read(uint8_t device_addr, uint8_t reg_addr)
{
	uint8_t data;

	i2c_Start();
	
	i2c_SendByte(device_addr | 0x0);
	if (i2c_WaitAck() != 0)
	{
		PRINT_EMG("%s-%d\n", __func__, __LINE__);
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	i2c_SendByte(reg_addr);
	if (i2c_WaitAck() != 0)
	{
		PRINT_EMG("%s-%d\n", __func__, __LINE__);
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	i2c_Start();
	i2c_SendByte(device_addr | 0x1); /* READ */
	
	if (i2c_WaitAck() != 0)
	{
		PRINT_EMG("%s-%d\n", __func__, __LINE__);
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	data = i2c_ReadByte();
	PRINT_EMG("read data: [0x%x]: 0x%x \n", reg_addr, data);
	i2c_Ack();
	
	i2c_Stop();
	
	return data;
	
cmd_fail:
	PRINT_EMG("cmd fail!\n");
	return 1;

}

uint8_t i2c_write(uint8_t device_addr, uint8_t addr, uint8_t data)
{

    i2c_Start();

    i2c_SendByte(device_addr | 0x0);
    if (i2c_WaitAck() != 0)
    {
        PRINT_EMG("%s-%d\n", __func__, __LINE__);
        goto cmd_fail;
    }

    i2c_SendByte(addr);
    if (i2c_WaitAck() != 0)
    {
        PRINT_EMG("%s-%d\n", __func__, __LINE__);
        goto cmd_fail;
    }

    i2c_SendByte(data);
    if (i2c_WaitAck() != 0)
    {
        PRINT_EMG("%s-%d\n", __func__, __LINE__);
        goto cmd_fail;
    }
	
	i2c_Stop();
	
	PRINT_EMG("%s write [0x%x]:0x%x\n", __func__, addr, data);
    return data;

cmd_fail:
	i2c_Stop();
    PRINT_EMG("cmd fail!\n");
    return 1;

}

uint32_t i2c_am2321_dump()
{
	/* step 1. wakeup am2321 */
	i2c_Start();
    i2c_SendByte(AM2321_ADDR | 0x0);

	mdelay(1);
	i2c_Stop();
	
	/* step 2. send request */
	i2c_Start();
	
    i2c_SendByte(AM2321_ADDR | 0x0);
    if (i2c_WaitAck() != 0)
    {
        PRINT_EMG("%s-%d\n", __func__, __LINE__);
        goto cmd_fail;
    }
	
    i2c_SendByte(0x03);
    if (i2c_WaitAck() != 0)
    {
        PRINT_EMG("%s-%d\n", __func__, __LINE__);
        goto cmd_fail;
    }
	
    i2c_SendByte(0x00);
    if (i2c_WaitAck() != 0)
    {
        PRINT_EMG("%s-%d\n", __func__, __LINE__);
        goto cmd_fail;
    }
	
    i2c_SendByte(0x04);
    if (i2c_WaitAck() != 0)
    {
        PRINT_EMG("%s-%d\n", __func__, __LINE__);
        goto cmd_fail;
    }

	i2c_Stop();

	mdelay(2);
	
	return 0;
cmd_fail:
	i2c_Stop();
	PRINT_EMG("cmd fail!\n");
	return 1;

}

uint32_t i2c_test()
{
	uint8_t data;
	
	/* EEPROM */
	if (i2c_checkdevice(0xA0) == 0)
	{
		PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
		ee_ReadByte(0xA0, 0x0);
		ee_ReadByte(0xA0, 0x1);
		ee_ReadByte(0xA0, 0x2);

		i2c_write(0xA0, 0x0, 0xFA);
		mdelay(1000);
		i2c_read(0xA0, 0x0);
#if 0
		i2c_read(0xA0, 0x0);
		mdelay(100);
		i2c_read(0xA0, 0x1);
		mdelay(100);
		i2c_read(0xA0, 0x2);
#endif
		//return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
		i2c_Stop(); 	
		//return 0;
	}
	//return 0;
	//i2c_am2321_dump();

	if (i2c_checkdevice(AM2321_ADDR) == 0)
	{
		PRINT_EMG("%s-%d succ\n", __func__, __LINE__);	
		i2c_am2321_dump();
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
		i2c_Stop();		
		//return 0;
	}
	
	if (i2c_checkdevice(TSL2561_ADDR) == 0)
	{
		PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
		i2c_read(TSL2561_ADDR, 0xA); /* ID register */
		//return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
		i2c_Stop(); 	
		//return 0;
	}

	if (i2c_checkdevice(0xEE) == 0)	/* BMP180 read 0xEF write 0xEE */
	{
		PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
		//ee_ReadByte(0xEE, 0xD0);
		i2c_read(0xEE, 0xD0); /* 0x55 expected */
		i2c_Stop();
		i2c_read(0xEE, 0xAA);
		{
			uint16_t AC1, AC2, AC3, AC4, AC5, AC6;
			uint16_t B1, B2;
			uint16_t MB, MC, MD;
			
			mdelay(500);
			
			AC1 = i2c_read(0xEE, 0xAA) << 8 | i2c_read(0xEE, 0xAB);
			AC2 = i2c_read(0xEE, 0xAC) << 8 | i2c_read(0xEE, 0xAD);
			AC3 = i2c_read(0xEE, 0xAE) << 8 | i2c_read(0xEE, 0xAF);
			AC4 = i2c_read(0xEE, 0xB0) << 8 | i2c_read(0xEE, 0xB1);
			AC5 = i2c_read(0xEE, 0xB2) << 8 | i2c_read(0xEE, 0xB3);
			AC6 = i2c_read(0xEE, 0xB4) << 8 | i2c_read(0xEE, 0xB5);
			
			B1  = i2c_read(0xEE, 0xB6) << 8 | i2c_read(0xEE, 0xB7);
			B2  = i2c_read(0xEE, 0xB8) << 8 | i2c_read(0xEE, 0xB9);

			MB  = i2c_read(0xEE, 0xBA) << 8 | i2c_read(0xEE, 0xBB);
			MC  = i2c_read(0xEE, 0xBC) << 8 | i2c_read(0xEE, 0xBD);
			MD  = i2c_read(0xEE, 0xBE) << 8 | i2c_read(0xEE, 0xBF);

			DUMP_VAR(AC1);
			DUMP_VAR(AC2);
			DUMP_VAR(AC3);
			DUMP_VAR(AC4);
			DUMP_VAR(AC5);
			DUMP_VAR(AC6);

			DUMP_VAR(B1);
			DUMP_VAR(B2);

			DUMP_VAR(MB);
			DUMP_VAR(MC);
			DUMP_VAR(MD);
			
		}
		//return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
		i2c_Stop(); 	
		//return 0;
	}

	if (i2c_checkdevice(0xEF) == 0)	/* BMP180 read 0xEF write 0xEE */
	{
		PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
		i2c_read(0xEE, 0xD0);
		//return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
		i2c_Stop(); 	
		//return 0;
	}

	if (i2c_checkdevice(0xB8) == 0)	/* AM2312 */
	{
		PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
		i2c_Start();
		i2c_SendByte(0xB8);
		if (i2c_WaitAck() != 0)
		{
			PRINT_EMG("%s-%d\n", __func__, __LINE__);
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
		i2c_SendByte(0x00);
		i2c_Start();
		i2c_SendByte(0xB9);
		if (i2c_WaitAck() != 0)
		{
			PRINT_EMG("%s-%d\n", __func__, __LINE__);
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
		data = i2c_ReadByte();
		PRINT_EMG("data: 0x%x \n", data);
		i2c_Ack();
		i2c_Stop();

		//return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
		i2c_Stop(); 	
		//return 0;
	}

if (i2c_checkdevice(0xEE) == 0) /* BMP180 */
{
	PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
	i2c_Start();
	i2c_SendByte(0xEE);
	if (i2c_WaitAck() != 0)
	{
		PRINT_EMG("%s-%d\n", __func__, __LINE__);
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	i2c_SendByte(0xD0);
	i2c_Start();
	i2c_SendByte(0xEF);
	if (i2c_WaitAck() != 0)
	{
		PRINT_EMG("%s-%d\n", __func__, __LINE__);
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	data = i2c_ReadByte();
	PRINT_EMG("data: 0x%x \n", data);
	i2c_Ack();
	i2c_Stop();
	//return 1;
}
else
{
	/* 失败后，切记发送I2C总线停止信号 */
	PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
	i2c_Stop(); 	
	//return 0;
}

if (i2c_checkdevice(0x29) == 0)
{
	PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
	//return 1;
}
else
{
	/* 失败后，切记发送I2C总线停止信号 */
	PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
	i2c_Stop();
	//return 0;
}


if (i2c_checkdevice(0x49) == 0)
{
	PRINT_EMG("%s-%d succ\n", __func__, __LINE__);
	//return 1;
}
else
{
	/* 失败后，切记发送I2C总线停止信号 */
	PRINT_EMG("%s-%d fail\n", __func__, __LINE__);
	i2c_Stop();
	//return 0;
}

return 0;
cmd_fail:
	PRINT_EMG("cmd fail!\n");
	return 1;
}
