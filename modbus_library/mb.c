#include <stdio.h>
#include <stdlib.h>
#include "../types.h"
#include "mb.h"
#include "../drivers/uartDriver.h"
#include "../drivers/timerDriver.h"
#include "../functions/functions.h"
#include "crc.h"
#include <assert.h>

Timer_HandleTypeDef tmr;
Uart_HandleTypeDef uart;

uint16_t mbBufferPosition;
uint8_t mbBuffer[MB_SIZE_MAX];
uint8_t mbDevAddr;
MB_StateTypeDef mbState = MB_STATE_NOT_INITIALIZED;
MB_TsvStateTypeDef txState;
MB_RcvStateTypeDef rxState;
MB_EventTypeDef mbEvent;
uint8_t *pSndFrameCur;
uint16_t sndFrameCount;

MB_ErrorTypeDef receiveFrame(uint8_t *pSlaveAddr, uint8_t **ppPdu, uint16_t *pPduLength);       //Calculate destination address,
                                                                                                //PDU start, PDU length
MB_ErrorTypeDef sendFrame(uint16_t pduLength);                                                  //Send data from mbDuffer
void MB_byte_received_cb(uint8_t chr);                                                          //byte received callback
void MB_byte_sent_cb();                                                                         //byte sent callback
void MB_t15Expired_cb();                                                                        //t15 callback
void MB_t35Expired_cb();                                                                        //t35 callback

MB_ErrorTypeDef MB_init(uint8_t devAddr, uint32_t baudRate, uint32_t cpuFrequency)
{
MB_ErrorTypeDef error = MB_ERROR_OK;
  if ((devAddr < MB_ADDR_MIN) || (devAddr > MB_ADDR_MAX) || (baudRate == 0)) {
    error = MB_ERROR_ILLVAL;
  }
  else {
    mbDevAddr = devAddr;

    uart.baudRate = baudRate;
    uart.cpuFrequency = cpuFrequency;
    uart.byte_received_cb = MB_byte_received_cb;
    uart.byte_sent_cb = MB_byte_sent_cb;
    uartInit(&uart);
    printf("UART initialized at %u bod\n", (unsigned int)uart.baudRate);

    tmr.baudRate = uart.baudRate;
    tmr.cpuFrequency = cpuFrequency;
    tmr.t15_cb = MB_t15Expired_cb;
    tmr.t35_cb = MB_t35Expired_cb;
    timerInit(&tmr);

    mbState = MB_STATE_DISABLED;
    rxState = RX_STATE_IDLE;
    txState = TX_STATE_IDLE;
  }
  return error;
}

MB_ErrorTypeDef MB_start()
{
MB_ErrorTypeDef error = MB_ERROR_OK;
  if (mbState == MB_STATE_DISABLED) {
    mbState = MB_STATE_ENABLED;
    mbEvent = MB_EVENT_NONE;
    rxState = RX_STATE_INIT;
    txState = TX_STATE_IDLE;
    timerStartT35();
    UART_enable(true, false);
    error = MB_ERROR_OK;
  }
  else {
    error = MB_ERROR_ILLSTATE;
  }
  return error;
}

MB_ErrorTypeDef MB_stop()
{
MB_ErrorTypeDef error = MB_ERROR_OK;
  if (mbState == MB_STATE_ENABLED) {
    mbState = MB_STATE_DISABLED;
    rxState = RX_STATE_IDLE;
    Timer_stop();
    UART_enable(false, false);
    error = MB_ERROR_OK;
  }
  else {
    error = MB_ERROR_ILLSTATE;
  }
  return error;
}

void MB_byte_received_cb(uint8_t chr)
{
  //printf("%x ", chr);
  if (txState != TX_STATE_IDLE) {
    return;
  }
  switch(rxState) {
    case RX_STATE_INIT :                                        //Wait 3.5 chars, required for ModBus initialization
      timerStartT35();
      break;
    case RX_STATE_IDLE :                                        //First byte of Serial PDU received
      mbBufferPosition = 0;
      mbBuffer[mbBufferPosition++] = chr;
      rxState = RX_STATE_RCV;
      timerStartT35();
      break;
    case RX_STATE_RCV :                                         //Next byte of Serial PDU received
      if (mbBufferPosition < MB_SIZE_MAX + 1) {
        mbBuffer[mbBufferPosition++] = chr;                     //Save byte to buffer
      }
      else {                                                    //Buffer overflow occurred
        mbEvent = MB_EVENT_ERROR;
        rxState = RX_STATE_INIT;
      }
      timerStartT35();
      break;
    case RX_STATE_T35EXPECTED :                                 //t15 interval passed from last byte, but t35 not
      mbEvent = MB_EVENT_ERROR;
      rxState = RX_STATE_INIT;
      break;
 }
}

void MB_byte_sent_cb()
{
  if (rxState != RX_STATE_IDLE) {                               //Line is busy
    return;
  }
  switch (txState) {
    case TX_STATE_IDLE:                                         //Should not be called in normal operation
      break;
    case TX_STATE_TMIT:                                         //Transmit next byte
      if (sndFrameCount) {                                      //There is next byte
        UART_putByte(*pSndFrameCur);
        sndFrameCount--;
        pSndFrameCur++;
      }
      else {                                                    //Last byte transmitted just
        mbEvent = MB_EVENT_SENT;
        UART_enable(true, false);
        txState = TX_STATE_IDLE;
      }
  }
}

void MB_t15Expired_cb()
{
  if (rxState == RX_STATE_RCV)
    rxState = RX_STATE_T35EXPECTED;

}

void MB_t35Expired_cb()
{
  switch (rxState) {
    case RX_STATE_INIT :                                          //ModBus initialization finished
      mbEvent = MB_EVENT_READY;
      break;
    case RX_STATE_RCV :                                           //Abnormal way to complete frame receiving
      mbEvent = MB_EVENT_ERROR;
      break;
    case RX_STATE_IDLE :
      break;
    case RX_STATE_T35EXPECTED :                                   //Normal way to complete frame receiving
      mbEvent = MB_EVENT_FRAME_RECEIVED;
      break;
  }
  //printf("\n");
  rxState = RX_STATE_IDLE;
}

MB_ErrorTypeDef MB_poll()
{
static uint8_t destAddr;
static uint8_t *pPdu;
static uint16_t pduSize;
static uint8_t functionCode;
static MB_ExceptionTypeDef excep;

  if (mbState != MB_STATE_ENABLED)
    return MB_ERROR_ILLSTATE;

  if (mbEvent != MB_EVENT_NONE)
  {
    switch (mbEvent) {
      case MB_EVENT_READY :                                                   //t35 pause after Start passed
        mbEvent = MB_EVENT_NONE;
        break;
      case MB_EVENT_ERROR :                                                   //Error occurred
        mbEvent = MB_EVENT_NONE;
        break;
      case MB_EVENT_FRAME_RECEIVED :                                          //Frame received
        if (receiveFrame(&destAddr, &pPdu, &pduSize) == MB_ERROR_OK) {        //CRC and length correct
          if ((destAddr == mbDevAddr) || (destAddr == MB_ADDR_BROADCAST)) {   //Destination address is own
            mbEvent = MB_EVENT_EXECUTE;
          }
          else {                                                              //Destination address is wrong
            mbEvent = MB_EVENT_NONE;
          }
        }
        else {                                                                //Frame CRC or length failed
          mbEvent = MB_EVENT_ERROR;
        }
        break;
      case MB_EVENT_EXECUTE :                                                 //Valid frame with own address received
        functionCode = *(pPdu + PDU_FUNC_OFFSET);
        if (functionCode == MB_FUNC_WRITE_REGISTER) {                         //Function #06
          excep = APP_WriteHoldingRegister(pPdu, &pduSize);
        }
        else if (functionCode == MB_FUNC_READ_REGISTERS) {                    //Function #03
          excep = APP_ReadHoldingRegisters(pPdu, &pduSize);
        }
        else {                                                                //Unsupported function
          excep = MB_EXCEP_ILLFUNCTION;
        }

        mbEvent = MB_EVENT_NONE;
        if (destAddr != MB_ADDR_BROADCAST) {                                  //Destination address is not broadcast
          if (excep != MB_EXCEP_NONE) {                                       //Build exception response
            pduSize = 0;
            *(pPdu + pduSize++) = functionCode | MB_FUNC_ERROR;
            *(pPdu + pduSize++) = excep;
          }
          if(sendFrame(pduSize) != MB_ERROR_OK) {                             //Send normal or exception response
            mbEvent = MB_EVENT_ERROR;                                         //UART transmitter error
          }
        }
        break;
      case MB_EVENT_SENT :                                                    //All frame sent
        mbEvent = MB_EVENT_NONE;
        break;
      case MB_EVENT_NONE :
        break;
    }
  }
  return MB_ERROR_OK;
}

MB_ErrorTypeDef receiveFrame(uint8_t *pSlaveAddr, uint8_t **ppPdu, uint16_t *pPduLength)
{
  if ((mbBufferPosition < MB_SIZE_MIN) || (mbBufferPosition > MB_SIZE_MAX)) {
    return MB_ERROR_IOFAIL;
  }

  if (getCrc16(mbBuffer, mbBufferPosition) != 0) {
    return MB_ERROR_IOFAIL;
  }

  *pSlaveAddr = *(mbBuffer + MB_ADDR_OFFSET);
  *pPduLength = mbBufferPosition - MB_CRC_LENGTH - MB_ADDR_LENGTH;
  *ppPdu = mbBuffer + MB_ADDR_LENGTH;
  return MB_ERROR_OK;
}

MB_ErrorTypeDef sendFrame(uint16_t pduLength)
{
MB_ErrorTypeDef error = MB_ERROR_OK;
uint16_t crc16;
  if (rxState == RX_STATE_IDLE) {
    pSndFrameCur = mbBuffer;
    sndFrameCount = MB_ADDR_LENGTH + pduLength;
    if (sndFrameCount <= MB_SIZE_MAX) {
      crc16 = getCrc16(pSndFrameCur, sndFrameCount);
      *(pSndFrameCur + sndFrameCount++) = (uint8_t)(crc16 & 0xFF);
      *(pSndFrameCur + sndFrameCount++) = (uint8_t)(crc16 >> 8);
      txState = TX_STATE_TMIT;
      UART_enable(false, true);
    }
    else error = MB_ERROR_ILLVAL;
  }
  else {
    error = MB_ERROR_IOFAIL;
  }
  return error;
}

