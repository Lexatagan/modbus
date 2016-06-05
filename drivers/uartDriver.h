#ifndef UARTDRIVER_H_INCLUDED
#define UARTDRIVER_H_INCLUDED

#define UART              ((UART_TypeDef *)UART_BASE)
#define UART_UCSR_RXEN    0x01
#define UART_UCSR_TXEN    0x02
#define UART_UCSR_FE      0x04

typedef struct                                                  //UART registers
{
  __IO uint32_t UDR;
  __IO uint32_t UBRR;
  __IO uint32_t UCSR;
} UART_TypeDef;

typedef struct                                                  //UART initialize structure
{
  uint32_t baudRate;                                            //Baud rate
  uint32_t cpuFrequency;                                        //CPU clock frequency
  void (*byte_received_cb)(uint8_t chr);                        //byte received callback
  void (*byte_sent_cb)(void);                                   //byte sent callback
} Uart_HandleTypeDef;

void uartInit(Uart_HandleTypeDef *uartHandleStruct);            //Initialize UART
void UART_enable(bool rxEnable, bool txEnable);                 //Enable/disable receiver/transmitter
void UART_putByte(uint8_t data);                                //Send byte
void UART_RXC_vect();                                           //UART Rx vector
void UART_TXC_vect();                                           //UART Tx vector

#endif // UARTDRIVER_H_INCLUDED
