#include "../types.h"
#include "../bsp.h"
#include "timerDriver.h"

void (*timer_cb)(void);

void timerStart(uint32_t periodTicks, void (*timerElapsed_cb)(void))
{
  TIMER->TCNT = periodTicks;
  TIMER->TCSR |= TIMER_TCSR_TEN;
  timer_cb = timerElapsed_cb;
}

void Timer_stop()
{
  TIMER->TCSR &= ~TIMER_TCSR_TEN;
  timer_cb = 0;
}

void Timer_vect()
{
  if (timer_cb != 0)
    timer_cb();
}
