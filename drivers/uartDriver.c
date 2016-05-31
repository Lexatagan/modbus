#include "../types.h"
#include "../bsp.h"
#include "uartDriver.h"

Uart_HandleTypeDef *huart;

void uartInit(Uart_HandleTypeDef *uartHandleStruct)
{
  if (uartHandleStruct == 0)
    return;
  huart = uartHandleStruct;
  UART_enable(false, false);
  UART->UBRR = (uint32_t)((CPU_FREQ/(huart->baudRate << 4) - 1));         //Calculate real baud rate
  huart->baudRate = (uint32_t)((CPU_FREQ/((UART->UBRR + 1) << 4)));
}

void UART_enable(bool rxEnable, bool txEnable)
{
  if (rxEnable)
    UART->UCSR |= UART_UCSR_RXEN;
  else
    UART->UCSR &= ~UART_UCSR_RXEN;
  if (txEnable)
    UART->UCSR |= UART_UCSR_TXEN;
  else
    UART->UCSR &= ~UART_UCSR_TXEN;
}

void UART_putByte(uint8_t data)
{
  UART->UDR = data;
}

void UART_RXC_vect()
{
  if (huart->byte_received_cb != 0)
    huart->byte_received_cb((uint8_t)UART->UDR);
}

void UART_TXC_vect()
{
  if (huart->byte_sent_cb != 0)
    huart->byte_sent_cb();
}
