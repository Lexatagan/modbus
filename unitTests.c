#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"
#include "bsp.h"
#include "unitTests.h"
#include "modbus_library/mb.h"
#include "drivers/uartDriver.h"
#include "drivers/timerDriver.h"

// ModBus variables
extern MB_StateTypeDef mbState;
extern MB_TsvStateTypeDef txState;
extern MB_RcvStateTypeDef rxState;
extern uint8_t mbDevAddr;
extern MB_EventTypeDef mbEvent;
extern uint16_t mbBufferPosition;
extern uint8_t *pSndFrameCur;
extern uint16_t sndFrameCount;

//Driver variables
extern __IO uint32_t uartReg[];
extern __IO uint32_t timerReg[2];
extern uint8_t mbBuffer[];
extern uint16_t t15;
extern uint16_t t35;
extern uint16_t t15Cnt;
extern uint16_t t35Cnt;
extern uint16_t memCard[];

MB_ErrorTypeDef receiveFrame(uint8_t *pSlaveAddr, uint8_t **frame, uint16_t *pFrameLength);
MB_ErrorTypeDef sendFrame(uint16_t pduLength);
void MB_t35Expired_cb();                                                                        //t35 callback
void MB_byte_received_cb(uint8_t chr);                                                          //byte received callback
void MB_byte_sent_cb();                                                                         //byte sent callback

void emulateDelay(uint16_t delay);
uint16_t compareBuffer(char *data, uint16_t length);
void fillBuffer(char *data, uint16_t length);
void emulateFrameReceive(char *data, uint16_t length);
void emulateFrameSent(uint16_t length);


void startUnitTests()
{
uint8_t addr;
uint8_t *pframe;
uint16_t framesize;

  printf("Starting unit tests...\n");

  //Examine MB_t35Expired_cb()
  rxState = RX_STATE_INIT;
  MB_t35Expired_cb();
  assert(mbEvent == MB_EVENT_READY);
  assert(rxState == RX_STATE_IDLE);

  rxState = RX_STATE_RCV;
  MB_t35Expired_cb();
  assert(mbEvent == MB_EVENT_ERROR);
  assert(rxState == RX_STATE_IDLE);

  rxState = RX_STATE_T35EXPECTED;
  MB_t35Expired_cb();
  assert(mbEvent == MB_EVENT_FRAME_RECEIVED);
  assert(rxState == RX_STATE_IDLE);

  //Examine MB_init()
  assert(MB_init(0x00, 9600, CPU_FREQ) == MB_ERROR_ILLVAL);             //Try invalid slave addresses
  assert(MB_init(0xF8, 9600, CPU_FREQ) == MB_ERROR_ILLVAL);
  assert(MB_init(MB_SLAVE_ADDR, 0, CPU_FREQ) == MB_ERROR_ILLVAL);       //Try 0 baud rate
  assert(MB_init(MB_SLAVE_ADDR, 38400, CPU_FREQ) == MB_ERROR_OK);       //Try valid configuration
  assert(mbDevAddr == 0x30);
  assert(mbState == MB_STATE_DISABLED);
  assert(rxState == RX_STATE_IDLE);
  assert(txState == TX_STATE_IDLE);

  //Examine MB_start()
  mbState = MB_STATE_NOT_INITIALIZED;
  assert(MB_start() == MB_ERROR_ILLSTATE);
  mbState = MB_STATE_ENABLED;
  assert(MB_start() == MB_ERROR_ILLSTATE);
  mbState = MB_STATE_DISABLED;
  assert(MB_start() == MB_ERROR_OK);
  assert(mbState == MB_STATE_ENABLED);
  assert(rxState == RX_STATE_INIT);
  assert((TIMER->TCSR & TIMER_TCSR_TEN) == TIMER_TCSR_TEN);   //Timer started
  assert((UART->UCSR & UART_UCSR_RXEN) == UART_UCSR_RXEN);    //UART receive enabled
  assert((UART->UCSR & UART_UCSR_TXEN) == 0);                 //UART transmit disabled

  //Examine MB_stop()
  mbState = MB_STATE_NOT_INITIALIZED;
  assert(MB_stop() == MB_ERROR_ILLSTATE);
  mbState = MB_STATE_DISABLED;
  assert(MB_stop() == MB_ERROR_ILLSTATE);
  mbState = MB_STATE_ENABLED;
  assert(MB_stop() == MB_ERROR_OK);
  assert(mbState == MB_STATE_DISABLED);
  assert(rxState == RX_STATE_IDLE);
  assert((TIMER->TCSR & TIMER_TCSR_TEN) == 0);                //Timer stopped
  assert((UART->UCSR & UART_UCSR_RXEN) == 0);                 //UART receive disabled
  assert((UART->UCSR & UART_UCSR_TXEN) == 0);                 //UART transmit disabled

  //Examine MB_byte_received_cb()
  MB_start();
  Timer_stop();

  rxState = RX_STATE_INIT;
  MB_byte_received_cb(0x00);                                  //Try receiving byte during rx in INIT
  assert((TIMER->TCSR & TIMER_TCSR_TEN) == TIMER_TCSR_TEN);
  assert(t35Cnt == t35);

  rxState = RX_STATE_T35EXPECTED;
  MB_byte_received_cb(0x00);
  assert(mbEvent == MB_EVENT_ERROR);

  rxState = RX_STATE_IDLE;
  MB_byte_received_cb(0x10);                                  //Receiving byte during rx in IDLE
  assert(t35Cnt == t35);
  assert(mbBufferPosition == 1);
  assert(rxState == RX_STATE_RCV);
  assert(mbBuffer[0] == 0x10);

  mbBufferPosition = 100;
  MB_byte_received_cb(0x20);                                  //Receiving byte during rx in RCV
  assert(mbBufferPosition == 101);
  assert(mbBuffer[100] == 0x20);

  mbBufferPosition = MB_SIZE_MAX + 1;
  MB_byte_received_cb(0x30);                                  //Receiving excessive byte
  assert(mbBufferPosition ==  MB_SIZE_MAX + 1);
  assert(mbEvent == MB_EVENT_ERROR);

  //Examine MB_byte_sent_cb()
  rxState = RX_STATE_IDLE;
  txState = TX_STATE_IDLE;
  MB_byte_sent_cb();
  assert(txState == TX_STATE_IDLE);

  txState = TX_STATE_TMIT;
  sndFrameCount = 100;
  pSndFrameCur = mbBuffer;
  MB_byte_sent_cb();
  assert(txState == TX_STATE_TMIT);
  assert(sndFrameCount == 99);
  assert(pSndFrameCur == mbBuffer + 1);

  sndFrameCount = 0;
  MB_byte_sent_cb();
  assert(txState == TX_STATE_IDLE);
  assert(mbEvent == MB_EVENT_SENT);
  assert((UART->UCSR & UART_UCSR_RXEN) == UART_UCSR_RXEN);    //UART receive enabled
  assert((UART->UCSR & UART_UCSR_TXEN) == 0);                 //UART transmit disabled

  //Examine MB_receiveFrame()
  fillBuffer("123456789\x37\x4b", 11);                                          //Try small 3-byte frame
  assert(receiveFrame(&addr, &pframe, &framesize) == MB_ERROR_OK);

  fillBuffer("", 3);                                          //Try small 3-byte frame
  assert(receiveFrame(&addr, &pframe, &framesize) == MB_ERROR_IOFAIL);

  fillBuffer("", MB_SIZE_MAX + 1);                            //Try big (MB_SIZE_MAX + 1)-byte frame
  assert(receiveFrame(&addr, &pframe, &framesize) == MB_ERROR_IOFAIL);

  fillBuffer("\x20\x00\x00\x00", 4);                          //Try valid frame
  assert(receiveFrame(&addr, &pframe, &framesize) == MB_ERROR_OK);
  assert(addr == 0x20);
  assert(pframe == mbBuffer + 1);
  assert(framesize == 1);

  //Examine MB_SendFrame()
  rxState = RX_STATE_RCV;
  assert(sendFrame(1) == MB_ERROR_IOFAIL);

  rxState = RX_STATE_IDLE;
  fillBuffer("\x00\x00\x00\x00",4);
  assert(sendFrame(1) == MB_ERROR_OK);
  assert(compareBuffer("\x00\x00\x00\x00",4) == 0);
  assert(pSndFrameCur == mbBuffer);
  assert(sndFrameCount == 4);
  assert(txState == TX_STATE_TMIT);

  //Examine MB_poll()
  mbState = MB_STATE_NOT_INITIALIZED;
  assert(MB_poll() == MB_ERROR_ILLSTATE);
  mbState = MB_STATE_DISABLED;
  assert(MB_poll() == MB_ERROR_ILLSTATE);

  mbState = MB_STATE_ENABLED;
  mbEvent = MB_EVENT_READY;
  MB_poll();
  assert(mbEvent == MB_EVENT_NONE);
  mbEvent = MB_EVENT_SENT;
  MB_poll();
  assert(mbEvent == MB_EVENT_NONE);
  mbEvent = MB_EVENT_NONE;
  MB_poll();
  assert(mbEvent == MB_EVENT_NONE);

  fillBuffer("", 3);                                          //Invalid frame
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  assert(mbEvent == MB_EVENT_ERROR);
  MB_poll();
  assert(mbEvent == MB_EVENT_NONE);

  fillBuffer("\x01\x00\x00\x00", 4);                          //Valid frame with wrong address
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  assert(mbEvent == MB_EVENT_NONE);

  rxState = RX_STATE_RCV;                                     //Emulate UART is receiving

  fillBuffer("\x00\x00\x00\x00", 4);                          //Valid frame with broadcast address and unsupported function
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  assert(mbEvent == MB_EVENT_EXECUTE);
  MB_poll();
  assert(mbEvent == MB_EVENT_NONE);

  fillBuffer("\x30\x00\x00\x00", 4);                          //Valid frame with own address and unsupported function
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  assert(mbEvent == MB_EVENT_EXECUTE);
  MB_poll();
  assert(mbEvent == MB_EVENT_ERROR);                          //Because of UART is receiving

  rxState = RX_STATE_IDLE;                                    //Emulate UART is not receiving

  fillBuffer("\x30\x00\x00\x00", 4);                          //Valid frame with own address and unsupported function
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  assert(mbEvent == MB_EVENT_EXECUTE);
  MB_poll();
  assert(mbEvent == MB_EVENT_NONE);
  assert(compareBuffer("\x30\x80\x01\x00\x00", 5) == 0);
  assert(txState = TX_STATE_TMIT);

  fillBuffer("\x30\x06\x10\x00\x44\x77\x00\x00", 8);           //Frame with valid function with invalid data
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  MB_poll();
  assert(compareBuffer("\x30\x86\x03\x00\x00", 5) == 0);


  txState = TX_STATE_IDLE;
  fillBuffer("\x00\x06\x00\x31\x33\x44\x00\x00", 8);          //Frame with broadcast address with valid data
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  MB_poll();
  assert(memCard[0x31] == 0x3344);
  assert(txState == TX_STATE_IDLE);

  txState = TX_STATE_IDLE;
  fillBuffer("\x30\x06\x00\x32\x55\x66\x00\x00", 8);          //Frame with own address with valid data
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  MB_poll();
  assert(memCard[0x32] == 0x5566);
  assert(txState == TX_STATE_TMIT);
  assert(compareBuffer("\x30\x06\x00\x32\x55\x66\x00\x00", 8) == 0);

  txState = TX_STATE_IDLE;
  fillBuffer("\x30\x03\x00\x38\x10\x02\x00\x00", 8);          //Frame with own address with invalid data
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  MB_poll();
  assert(txState == TX_STATE_TMIT);
  assert(compareBuffer("\x30\x83\x03\x00\x00", 5) == 0);

  txState = TX_STATE_IDLE;
  fillBuffer("\x30\x03\x00\x31\x00\x02\x00\x00", 8);          //Frame with own address with valid data
  mbEvent = MB_EVENT_FRAME_RECEIVED;
  MB_poll();
  MB_poll();
  assert(txState == TX_STATE_TMIT);
  assert(compareBuffer("\x30\x03\x04\x33\x44\x55\x66\x00\x00", 9) == 0);
  MB_stop();
  printf("Unit tests successfully finished.\n");

  //Complex examine
  MB_stop();                                                  //Restart ModBus
  MB_start();
  emulateDelay(t35);
  assert(mbEvent == MB_EVENT_READY);
  MB_poll();
  emulateFrameReceive("\x30\x03\x00\x31\x00\x02\x00\x00", 8);
  assert(compareBuffer("\x30\x03\x00\x31\x00\x02\x00\x00", 8) == 0);
  assert(rxState == RX_STATE_RCV);
  assert(mbEvent == MB_EVENT_NONE);
  emulateDelay(t15);
  assert(rxState == RX_STATE_T35EXPECTED);
  assert(mbEvent == MB_EVENT_NONE);
  emulateDelay(t35 - t15);
  assert(rxState == RX_STATE_IDLE);
  assert(mbEvent == MB_EVENT_FRAME_RECEIVED);
  assert(txState == TX_STATE_IDLE);
  MB_poll();
  MB_poll();
  assert(compareBuffer("\x30\x03\x04\x33\x44\x55\x66\x00\x00", 9) == 0);
  assert(txState == TX_STATE_TMIT);
  emulateFrameSent(10);
  assert(txState == TX_STATE_IDLE);
}

void emulateDelay(uint16_t delay)
{
uint8_t i;
  for (i = 0; i < delay; i++) {
    Timer_vect();
  }
}

uint16_t compareBuffer(char *data, uint16_t length)               //Returns 0, if rxBuffer and *data are equal
{
uint16_t i;
  for (i = 0; i < length; i++) {
    if (*(mbBuffer + i) != (uint8_t)*data++) {
      return i;
    }
  }
  return 0;
}

void fillBuffer(char *data, uint16_t length)                      //Fills rxBuffer with *data
{
uint16_t i;
  for (i = 0; i < length; i++) {
    *(mbBuffer + i) = *(data++);
  }
  mbBufferPosition = length;
}

void emulateFrameReceive(char *data, uint16_t length)
{
uint16_t i;
  for (i = 0; i < length; i++) {
    MB_byte_received_cb(*(data + i));
  }
}

void emulateFrameSent(uint16_t length)
{
uint16_t i;
  for (i = 0; i < length; i++) {
    MB_byte_sent_cb();
  }
}

