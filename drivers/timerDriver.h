#ifndef TIMERDRIVER_H_INCLUDED
#define TIMERDRIVER_H_INCLUDED

typedef struct
{
  __IO uint32_t TCNT;
  __IO uint32_t TCSR;
} Timer_TypeDef;

#define TIMER_TCSR_TEN              0x01

#define TIMER                       ((Timer_TypeDef *)TIMER_BASE)

void timerStart(uint32_t periodTicks, void (*timerElapsed_cb)(void));
void Timer_stop();
void Timer_vect();

#endif // TIMERDRIVER_H_INCLUDED
