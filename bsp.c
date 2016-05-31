#include "types.h"
#include "bsp.h"

uint16_t memCard[MC_SIZE];                    //Emulation of memory card
__IO uint32_t uartReg[3] = {0, 0, 0};         //Emulation of UART registers
__IO uint32_t timerReg[2] = {0, 0};           //Emulation of Timer registers

