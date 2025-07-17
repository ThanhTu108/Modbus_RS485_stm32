#include "master_slave.h"

//your uart you want
extern UART_HandleTypeDef huart3;
uint16_t ModbusRegister_Slave[Number_of_register];
uint16_t ModbusRegister_Master[Number_of_register];
bool ModbusCoil[Number_of_coil];

uint8_t uartRxData;
uint8_t DataCounter;
uint8_t RxInterruptFlag;
uint8_t uartTimeCounter;
uint8_t uartPacketComplatedFlag;

extern Modestate_t mode;
char ModbusRx[Buffersize];
char tempModbusRx[Buffersize];
char ModbusTx[Buffersize];


//Calculate crc
uint16_t Modbus_Crc(char *buf, uint8_t len)
{
	static const uint16_t table[2] = {0x0000, 0xA001};
	uint16_t crc = 0xFFFF;
	unsigned int i = 0, xor = 0;
	uint8_t bit = 0;
	
	for(i = 0; i < len; i++)
	{
		crc ^=buf[i];
		for(bit = 0; bit < 8; bit ++)
		{
			xor = crc & 0x01;
			crc >>= 1;
			crc ^=table[xor];
		}
	}
	return crc;
}


//Send message through uart
funcstate_t SendMessage(char *msg, uint16_t len)
{
	Uartstate_t Tx_state;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	Tx_state = (Uartstate_t)HAL_UART_Transmit(&huart3,(uint8_t*)msg, len,100);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	if(Tx_state == uartOk)
	{
		return funcOk;
	}
	else 
	{
		return funcErr;
	}
}

//Receive message through uart
funcstate_t ReceiveMessage(char *msg, uint16_t len, uint8_t Slaveid)
{
  uint16_t CRC_rx, CRC_val;
	uint16_t tempCounter;
  if (uartPacketComplatedFlag == RESET)
	{
		return funcErr;
	}
	tempCounter = DataCounter;	
	memcpy(msg, ModbusRx, DataCounter + 1);
	DataCounter = 0;
		
	//	memset(msg, 0, Buffersize);             
	//	memcpy(msg, ModbusRx, tempCounter); 
		
	memset(ModbusRx, 0, Buffersize); 
	//CRC check 
	CRC_rx = msg[tempCounter - 1] <<8| (msg[tempCounter-2]);
	CRC_val = Modbus_Crc(msg, tempCounter - 2);
	uartPacketComplatedFlag = RESET;
	if (CRC_val == CRC_rx && msg[0] == Slaveid)
	{
		return funcOk;
	}
	else
	{
		return funcErr;	
	}
}
	

void transmitDataMake(char *msg, uint8_t Lenght)
{
	switch(msg[1])
	{
	case ReadCoil:
//		makePacket_01(msg, Lenght);
		break;

	case ReadHoldingRegister:
		makePacket_03(msg, Lenght);
		break;

	case WriteSingleRegister:
		makePacket_06(msg, Lenght);
		break;
	}
}

void uartDataHandler(void)
{
	uint8_t tempCounter;
	uint16_t CRCValue;
	uint16_t rxCRC;
	if(mode == S_Mode && uartPacketComplatedFlag == SET)     //Data receiving is finished
	{
		uartPacketComplatedFlag = RESET;
	  memcpy(tempModbusRx, ModbusRx, DataCounter + 1);
	  tempCounter = DataCounter;
		DataCounter = 0;
//		memset(ModbusRx, 0, Buffersize);
		memset(ModbusTx, 0, Buffersize);
	
		/*CRC Check*/
		CRCValue = Modbus_Crc(tempModbusRx, tempCounter - 2);
		rxCRC = (tempModbusRx[tempCounter -1] << 8) | (tempModbusRx[tempCounter - 2]);

		
		/*If the calculated CRC value and the received CRC value are equal and the Slave ID is correct, respond to the receiving data.  */
		if(rxCRC == CRCValue && tempModbusRx[0] == Slave_id)
		{
			transmitDataMake(&tempModbusRx[0], tempCounter);
		}

	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if (huart->Instance == USART3)
	{
		RxInterruptFlag = SET;
		ModbusRx[DataCounter++] = uartRxData;
		if (DataCounter >= Buffersize)
		{
			DataCounter = 0;	
		}
		HAL_UART_Receive_IT(&huart3, &uartRxData, 1);	
		uartTimeCounter = 0;
	}
}

void uartTimer(void)
{
	if (RxInterruptFlag == SET)
	{
		if (uartTimeCounter++ >= 5) 
		{	
			uartPacketComplatedFlag = SET;
			RxInterruptFlag = RESET;
			uartTimeCounter = 0;
		}
	}
}

funcstate_t Modbus_ReadHoldingRegister(uint8_t Slaveid, uint16_t StartAddress, uint16_t NumberOfReg)
{
	uint16_t CRC_val = 0;
	funcstate_t Tx_state, Rx_state;
	uint8_t i,m = 0;
	
	
	char ModbusRx_master_03[Buffersize];
	char ModbusTx_master[8];
	
	memset(ModbusTx_master, 0,8);
	
	ModbusTx_master[0] = Slaveid;
	ModbusTx_master[1] = ReadHoldingRegister;
	ModbusTx_master[2] = StartAddress >> 8;
	ModbusTx_master[3] = (uint8_t)(StartAddress & 0x00FF);
	ModbusTx_master[4] = (uint8_t)(NumberOfReg >> 8)&0xFF;
	ModbusTx_master[5] = (uint8_t)(NumberOfReg & 0x00FF);
	CRC_val = Modbus_Crc(ModbusTx_master, 6);
	
	ModbusTx_master[6] =(uint8_t)(CRC_val & 0x00FF);
	ModbusTx_master[7] = CRC_val >> 8;
	
	Tx_state = SendMessage(&ModbusTx_master[0], 8);
	HAL_UART_Receive_IT(&huart3, &uartRxData, 1);

if(Tx_state == funcOk)
	{
		uint32_t timeout = HAL_GetTick() + 200; 
		while (HAL_GetTick() < timeout)
		{
			if (uartPacketComplatedFlag == SET)
				break;
		}
		Rx_state = ReceiveMessage(ModbusRx_master_03,(NumberOfReg * 2)+5, Slaveid);
		if(Rx_state == funcOk)
		{
			for(i = 0; i < NumberOfReg; i++)
			{
				ModbusRegister_Master[StartAddress+i] = (uint16_t)((ModbusRx_master_03[3 + m] << 8) | (ModbusRx_master_03[4 + m]));
				m +=2;
			}
		}
	}
	memset(ModbusTx, 0, Buffersize);
	return Rx_state;
}

funcstate_t Modbus_WriteSingleRegister(uint8_t SlaveID, uint16_t RegAddress, uint16_t RegValue)
{
	uint16_t CRC_val = 0;
	funcstate_t Tx_state, RxState;

	char ModbusRx_master_06[Buffersize];
	char ModbusTx_master[8];

	memset(ModbusTx_master, 0, 8);

	ModbusTx_master[0] = SlaveID;
	ModbusTx_master[1] = WriteSingleRegister;
	ModbusTx_master[2] = (uint8_t)(RegAddress >> 8);
	ModbusTx_master[3] = (uint8_t)(RegAddress & 0x00FF);
	ModbusTx_master[4] = (uint8_t)(RegValue >> 8);
	ModbusTx_master[5] = (uint8_t)(RegValue & 0x00FF);

	CRC_val = Modbus_Crc(ModbusTx_master, 6);

	ModbusTx_master[6] = (uint8_t)(CRC_val & 0x00FF);
	ModbusTx_master[7] = CRC_val >> 8;

	Tx_state = SendMessage(&ModbusTx_master[0], 8);

	if(Tx_state == funcOk)
	{
		uint32_t timeout = HAL_GetTick() + 200; 
		while (HAL_GetTick() < timeout)
		{
			if (uartPacketComplatedFlag == SET)
				break;
		}
		RxState = ReceiveMessage(ModbusRx_master_06, 8, SlaveID);
	}
	memset(ModbusTx, 0, Buffersize);
	return RxState; //CRC and ID is true.
}



void makePacket_03(char *msg, uint8_t Lenght)
{
	
	uint8_t i, m = 0;

	uint16_t RegAddress = 0;
	uint16_t NumberOfReg = 0;
	uint16_t CRCValue;
//	memset(ModbusTx, 0, Buffersize);
	RegAddress = (msg[2] << 8) | (msg[3]);
	NumberOfReg = (msg[4] << 8) | (msg[5]);
	ModbusTx[0] = msg[0];
	ModbusTx[1] = msg[1];
	ModbusTx[2] = (NumberOfReg * 2);

	for(i = 0; i < NumberOfReg * 2; i += 2)
	{
		ModbusTx[4 + i] = (uint8_t)(ModbusRegister_Slave[RegAddress + m] & 0x00FF);
		ModbusTx[3 + i] = (uint8_t)(ModbusRegister_Slave[RegAddress + m] >> 8);
		m++;
	}

	//CRC Calculate
	CRCValue = Modbus_Crc(ModbusTx, 3 + (NumberOfReg * 2 ));
	ModbusTx[4 + (NumberOfReg * 2 )] = (CRCValue >> 8);
	ModbusTx[3 + (NumberOfReg * 2 )] = (CRCValue & 0x00FF);
	
	/********************************************************/
	SendMessage(ModbusTx, 5 + (NumberOfReg * 2 ));
}

void makePacket_06(char *msg, uint8_t Lenght)
{
	uint16_t RegAddress, RegValue;
//	memset(ModbusTx, 0, Buffersize);
	RegAddress = (msg[2] << 8) | (msg[3]);
	RegValue = (msg[4] << 8) | (msg[5]);

	ModbusRegister_Slave[RegAddress] = RegValue;
	SendMessage(msg, Lenght);
}

