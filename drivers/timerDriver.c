#include "../types.h"
#include "../bsp.h"
#include "timerDriver.h"

Timer_HandleTypeDef *htmr;

uint16_t  t35 = 0;                                            //Corresponding to 3.5 char delay
uint16_t  t15 = 0;                                            //Corresponding to 1.5 char delay
uint16_t  t35Cnt = 0;
uint16_t  t15Cnt = 0;
uint16_t  timerPeriod;                                        //Timers period us

void timerStart(uint32_t periodus);

void timerInit(Timer_HandleTypeDef *timerHandleStruct)
{
  if (timerHandleStruct == 0)
    return;
  htmr = timerHandleStruct;
  if (htmr->baudRate > MODBUS_BIG_BAUDRATE)                   //For baud rates more than 19200 fixed intervals used (1750us, 750us)
  {
    t35 = T35;
    t15 = T15;
  }
  else                                                        //else calculate these intervals
  {
    t35 = (uint16_t)((7 * 11 * TIMER_IRQ_FREQUENCY)/(2 * htmr->baudRate));  //3.5 chars (11bit per char)
    t15 = (uint16_t)((3 * 11 * TIMER_IRQ_FREQUENCY)/(2 * htmr->baudRate));  //1.5 chars
  }
  timerPeriod = (uint16_t)(htmr->cpuFrequency / TIMER_IRQ_FREQUENCY);
}

void timerStart(uint32_t periodus)
{
  TIMER->TCNT = periodus;
  TIMER->TCSR |= TIMER_TCSR_TEN;
}

void Timer_stop()
{
  TIMER->TCSR &= ~TIMER_TCSR_TEN;
}

void timerStartT35()
{
  t35Cnt = t35;
  t15Cnt = t15;
  timerStart(timerPeriod);
}

void Timer_vect()
{
  if (t15Cnt > 0)
  {
    t15Cnt--;
    if (t15Cnt == 0)
    {
      if (htmr->t15_cb != 0)
        htmr->t15_cb();
    }
  }
  if (t35Cnt > 0)
  {
    t35Cnt--;
    if (t35Cnt == 0)
    {
      if (htmr->t35_cb != 0)
        htmr->t35_cb();
      Timer_stop();
    }
  }
}
