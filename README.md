## Example to use

# You must define Gpio DE/RE, Uart in lib.h and lib.c, enable nvic uart

*  ModbusRegister_Slave[0] = 200;
*  ModbusRegister_Slave[1] = 111;
*  ModbusRegister_Slave[2] = 111;			
*  HAL_UART_Receive_IT(&huart3, &uartRxData , 1);


*  while (1)
*  {
* 		if(mode == S_Mode)
* 		{
* 			uartDataHandler();
*		}
*		else if(mode == M_Mode)
*		{
*			Modbus_WriteSingleRegister(2, 3, 10);
*			Modbus_ReadHoldingRegister(2,0,5);
*		}
*		val[0] = ModbusRegister_Master[0];
*  }



  ** Put uartTimer() into systick_handler() in it.c
# If you only want use 1 function 
* (Slave: you should put uartDataHandler() in while true and define your id, mode = S_Mode in main)
* (Master: put any func like: Modbus_ReadHoldingRegister(slave id, start addr , number of register))
