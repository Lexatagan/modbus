#ifndef MCDRIVER_H_INCLUDED
#define MCDRIVER_H_INCLUDED

typedef enum
{
  ERROR_OK,
  ERROR_ILLVAL
} ErrorTypeDef;

ErrorTypeDef mcWriteRegister(uint16_t addr, uint16_t val);                      //Write 1 register
ErrorTypeDef mcReadRegisters(uint8_t *buffer, uint16_t addr, uint16_t size);    //Read multiple registers to buffer in Big-endian

#endif // MCDRIVER_H_INCLUDED
