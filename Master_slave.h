#ifndef __Master_slave_Modbus_485_H
#define __Master_slave_Modbus_485_H

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "main.h"
#include "string.h"

//Define your id
#define Slave_id 3
#define Number_of_register 10
#define Number_of_coil 16
#define Buffersize (Number_of_register * 2 + 5)

//#define Buffersize 128
extern uint16_t ModbusRegister_Slave[Number_of_register];
extern uint16_t ModbusRegister_Master[Number_of_register];
extern bool ModbusCoil[Number_of_coil];
extern uint8_t uartRxData;
extern uint8_t DataCounter;
extern uint8_t RxInterruptFlag;
extern uint8_t uartTimeCounter;
extern uint8_t uartPacketComplatedFlag;
extern char ModbusRx[Buffersize];
enum
{
	ReadCoil = 0x01,					
	ReadHoldingRegister = 0x03,	
	WriteSingleCoil = 0x05,		
	WriteSingleRegister = 0x06,	
	WriteMultipleCoils = 0x0F,	
	WriteMultipleRegisters = 0x10,		
};

typedef enum {
    uartOk,    
    uartError,  
} Uartstate_t;

typedef enum
{
	CrcOk,
	CrcErr,

} Crcstate_t;

typedef enum
{
	M_Mode,
	S_Mode,
}	Modestate_t;

typedef enum
{
	funcOk,
	funcErr,
	
} funcstate_t;

typedef enum
{
	coilOff = 0x00,
	coilOn = 0xff,
} coilState_t;


//Transmit and receive data
funcstate_t SendMessage(char *msg, uint16_t len);
funcstate_t ReceiveMessage_master(char *msg, uint16_t len, uint8_t Slaveid);

//Calculate CRC
uint16_t Modbus_Crc(char *buf, uint8_t len);
void uartTimer(void);

funcstate_t Modbus_ReadHoldingRegister(uint8_t Slaveid, uint16_t StartAddress, uint16_t NumberOfReg);
funcstate_t Modbus_WriteSingleRegister(uint8_t SlaveID, uint16_t RegAddress, uint16_t RegValue);
void transmitDataMake(char *msg, uint8_t Lenght);
void makePacket_06(char *msg, uint8_t Lenght);
void makePacket_03(char *msg, uint8_t Lenght);
void uartDataHandler(void);
#endif


