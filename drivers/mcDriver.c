#include "../types.h"
#include "mcDriver.h"
#include "../bsp.h"

ErrorTypeDef mcWriteRegister(uint16_t addr, uint16_t val)
{
  if ((addr >= MC_RWREGION_START) && (addr <= MC_RWREGION_END)) {             //If addr belongs to RW region
    *(MC_BASE + addr) = val;
    return ERROR_OK;
  }
  else {
    return ERROR_ILLVAL;
  }
}

ErrorTypeDef mcReadRegisters(uint8_t *buffer, uint16_t addr, uint16_t size)
{
uint16_t i;
  if ((addr >= MC_ROREGION_START) && (addr + size <= MC_ROREGION_END)) {      //If all region belongs to RO region
    for (i = 0; i < size; i++) {
      *buffer++ = (uint8_t)((*(MC_BASE + addr + i)) >> 8);                    //Hi byte
      *buffer++ = (uint8_t)((*(MC_BASE + addr + i)) & 0xFF);                  //Lo byte
    }
    return ERROR_OK;
  }
  else {
    return ERROR_ILLVAL;
  }
}
