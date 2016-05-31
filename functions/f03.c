#include "../types.h"
#include "../modbus_library/mb.h"
#include "../drivers/mcDriver.h"
#include "functions.h"

MB_ExceptionTypeDef APP_ReadHoldingRegisters(uint8_t *pPdu, uint16_t *pPduLength)
{
uint8_t registerAddress;
uint16_t registerCount;
uint8_t *pPduCur;
MB_ExceptionTypeDef excep = MB_EXCEP_NONE;
  if (*pPduLength == (PDU_SIZE_MIN + FUNC_READ_REGISTERS_SIZE)) {
    registerAddress = (uint16_t) (*(pPdu + FUNC_REGISTER_ADDR_OFFSET) << 8);
    registerAddress |= (uint16_t) (*(pPdu + FUNC_REGISTER_ADDR_OFFSET + 1));
    registerCount = (uint16_t) (*(pPdu + FUNC_REGISTER_VAL_OFFSET) << 8);
    registerCount |= (uint16_t) (*(pPdu + FUNC_REGISTER_VAL_OFFSET + 1));

    pPduCur = pPdu + PDU_DATA_OFFSET;
    *pPduCur++ = registerCount << 1;
    if ((mcReadRegisters(pPduCur, registerAddress, registerCount) == ERROR_OK) && (registerCount >= 1) \
        && (registerCount <= FUNC_READ_REGISTERS_MAX)) {
      *pPduLength = PDU_DATA_OFFSET + 1 + (registerCount << 1);
    }
    else {
      excep = MB_EXCEP_ILLVALUE;
    }
  }
  else {
    excep = MB_EXCEP_ILLVALUE;
  }
  return excep;
}
