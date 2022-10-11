/**********************************************************************
* Code : RS485 Modbus Fuctions File
* Devices: TMS320F28379D
* Reference Modbus Protocol Document
**********************************************************************/

Uint16 CRC_Calculator(Uint16 ModbusData[], Uint16 length)
{
    /* Steps for CRC Calculation: 
     * 1. Load a 16–bit register with FFFF hex (all 1’s). Call this the CRC register
     * 2. Exclusive OR the first 8–bit byte of the message with the low–order byte of the 16–bit CRC register,
     *    putting the result in the CRC register.
     * 3. Shift the CRC register one bit to the right (toward the LSB), zero–filling the MSB.
     *    Extract and examine the LSB
     * 4. (If the LSB was 0): Repeat Step 3 (another shift).
     *    (If the LSB was 1): Exclusive OR the CRC register with the polynomial value 0xA001 (1010 0000 0000 0001).
     * 5. Repeat Steps 3 and 4 until 8 shifts have been performed. When this is done,
     *    a complete 8–bit byte will have been processed.
     * 6. Repeat Steps 2 through 5 for the next 8–bit byte of the message.
     *    Continue doing this until all bytes have been processed.
     * 7. The final content of the CRC register is the CRC value.
     * 8. When the CRC is placed into the message, its upper and lower bytes must be swapped.
     */
    int m;
    CRC = 0xFFFF;

    for(m=0; m<length; m++)
    {
        CRC = CRC ^ ModbusData[m];
        CRC_Count = 0;
        while(CRC_Count<8)
        {
            flag = CRC & 1;
            CRC = CRC >> 1;
            CRC = CRC & 0x7FFF;
            if(flag == 1)
            {
                CRC = CRC ^ 0xA001;
            }
            CRC_Count++;
        }
    }
    return CRC;
}

float32 Modbus_ReadByte(Uint16 slaveAddr, Uint16 funCode, Uint16 dataAddr, Uint16 regLen)
{
    union u_tag {
        Uint16 b[2];
        float fval;
    } u;
    Uint16 dataAddrOriginal, regLenOriginal, modbusTxLen = 8, modbusRxLen = 9, returnValueReq,returnValueRes;
    Uint16 dataAddrHi, dataAddrLo, regLenHi, regLenLo, ModbusFailure;
    float32 tempVal,ModbusDataSend;

    dataAddrOriginal = dataAddr;
    dataAddrHi = dataAddr >> 8;
    dataAddrLo  = dataAddrOriginal & 0x00FF;

    regLenOriginal = regLen;
    regLenHi = regLen >> 8;
    regLenLo  = regLenOriginal & 0x00FF;

    Modbus_TxData[0] = slaveAddr;
    Modbus_TxData[1] = funCode;
    Modbus_TxData[2] = dataAddrHi;
    Modbus_TxData[3] = dataAddrLo;
    Modbus_TxData[4] = regLenHi;
    Modbus_TxData[5] = regLenLo;

    returnValueReq = CRC_Calculator(Modbus_TxData, modbusTxLen - 2);

    Modbus_TxData[6] = (returnValueReq & 0x00FF);
    Modbus_TxData[7] = (returnValueReq >> 8) & 0x00FF;

    /* Request of Modbus
     * Example:
     * 11 04 0008 0001 B298
     * 11: Slave Address
     * 04: Function Code
     * 0008: The Data Address of the first register requested
     * ( 0008 hex = 8 , + 30001 offset = input register #30009 )
     * 0001: The total number of registers requested. (read 1 register)
     * B298: The CRC(cyclic redundancy check) for error checking.
     */

    DriverEnableON = 1;
    ScicRegs.SCICTL1.bit.RXENA = 0;                                 //Disable Receiver
    Delay_ms(1);
    for(i = 0; i < modbusTxLen; i++)                                //Request Data from Energy Meter
    {
        while (ScicRegs.SCICTL2.bit.TXEMPTY == 0);                  //Check Transmit buffer empty
        ScicRegs.SCITXBUF.bit.TXDT = Modbus_TxData[i];              //Transmit Data
    }

    Delay_ms(2);
    ScicRegs.SCICTL1.bit.RXENA = 1;                                 //Enable Receiver
    DriverEnableOFF = 1;
    i=0;
    /* Response of Modbus
     * Example:
     * 11 04 02 000A F8F4
     * 11: Slave Address
     * 04: Function Code
     * 02: Number of data bytes to follow (1 registers x 2 bytes each = 2 bytes)
     * 000A: Contents of register 30009
     * F8F4: The CRC(cyclic redundancy check) for error checking.
     */
    ModbusDataSend = 0.0;
    DriverEnableOFF = 1;
    ScicRegs.SCICTL1.bit.RXENA = 1;                                 //Enable Receiver
    while(1)
    {
        while(ScicRegs.SCIFFRX.bit.RXFFINT == 0);                   //Wait for Receive interrupt Flag 
        Modbus_RxData[i]  = ScicRegs.SCIRXBUF.bit.SAR;              //Read Receive Buffer
        
        ScicRegs.SCIFFRX.bit.RXFFINTCLR  = 1;                       // clear RX-FIFO INT Flag
      
        i++;
        if(i == modbusRxLen)
        {
            i=0;
            break;
        }
    }

    returnValueRes = CRC_Calculator(Modbus_RxData, modbusRxLen - 2);
    if((Modbus_RxData[7] == (returnValueRes & 0x00FF)) && (Modbus_RxData[8] == ((returnValueRes >> 8) & 0x00FF)))
    {
        CRC_MatchFlag = 1;                                          //Check CRC for Error
    }

    //Manipulate data according to device used
    u.b[1] = Modbus_RxData[4] | (Modbus_RxData[3] << 8);
    u.b[0] = Modbus_RxData[6] | (Modbus_RxData[5] << 8);

    tempVal = u.fval;

    return tempVal;
}