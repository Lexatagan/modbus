#ifndef UARTDRIVER_H_INCLUDED
#define UARTDRIVER_H_INCLUDED

typedef struct
{
  uint32_t baudRate;
  uint32_t cpuFrequency;
  void (*t15_cb)(void);
  void (*t35_cb)(void);
} Timer_HandleTypeDef;

typedef struct
{
  __IO uint32_t TCNT;
  __IO uint32_t TCSR;
} Timer_TypeDef;

#define TIMER_TCSR_TEN              0x01

#define TIMER                       ((Timer_TypeDef *)TIMER_BASE)

#define MODBUS_BIG_BAUDRATE         19200                   //Fixed intervals used on big baud rates
#define TIMER_IRQ_FREQUENCY         20000                   //Optimal frequency of the timers interrupt
#define T15                         15                      //Timers periods corresponds 750us
#define T35                         35                      //Timers periods corresponds 1750us

void timerInit(Timer_HandleTypeDef *Timer_HandleStruct);
void timerStartT35();
void Timer_stop();
void Timer_vect();

#endif // UART_DRIVER_H_INCLUDED
