# Example to use
  ModbusRegister_Slave[0] = 200;
  ModbusRegister_Slave[1] = 111;
  ModbusRegister_Slave[2] = 111;
  ModbusRegister_Slave[3] = 0;
  ModbusRegister_Slave[4] = 103;
  ModbusRegister_Slave[5] = 6;
  ModbusRegister_Slave[6] = 4;
  ModbusRegister_Slave[7] = 5;
  ModbusRegister_Slave[8] = 6;
  ModbusRegister_Slave[9] = 4;

HAL_UART_Receive_IT(&huart3, &uartRxData , 1);

  while (1)
  {
		if(mode == S_Mode)
		{
			uartDataHandler();
		}
		else if(mode == M_Mode)
		{
			Modbus_WriteSingleRegister(2, 3, 10);
			Modbus_WriteSingleRegister(2, 3, 9);
			Modbus_WriteSingleRegister(2, 3, 8);
			Modbus_WriteSingleRegister(2, 3, 7);
			Modbus_WriteSingleRegister(2, 3, 6);
			Modbus_WriteSingleRegister(2, 3, 5);
			Modbus_WriteSingleRegister(2, 3, 4);
			Modbus_WriteSingleRegister(2, 3, 3);
			Modbus_ReadHoldingRegister(2,0,5);
		}
		val[0] = ModbusRegister_Master[0];
		val[1] = ModbusRegister_Master[1];
		val[2] = ModbusRegister_Master[2];
		val[3] = ModbusRegister_Master[3];
		val[4] = ModbusRegister_Master[4];
		val[5] = ModbusRegister_Master[5];
  }


  Put uartTimer() into systick_handler() in it.c
If you only want use 1 function 
(Slave: you should put uartDataHandler() in while true and define your id, mode = S_Mode in main)
(Master: put any func like: Modbus_ReadHoldingRegister(slave id, start addr , number of register))
