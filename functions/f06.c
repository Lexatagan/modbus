#include "../types.h"
#include "../modbus_library/mb.h"
#include "../drivers/mcDriver.h"
#include "functions.h"

MB_ExceptionTypeDef APP_WriteHoldingRegister (uint8_t *pPdu, uint16_t *pPduLength)
{
uint16_t registerAddress;
uint16_t registerValue;
MB_ExceptionTypeDef excep = MB_EXCEP_NONE;
  if (*pPduLength == (PDU_SIZE_MIN + FUNC_WRITE_REGISTER_SIZE)) {
    registerAddress = (uint16_t)(*(pPdu + FUNC_REGISTER_ADDR_OFFSET) << 8);
    registerAddress |= (uint16_t)(*(pPdu + FUNC_REGISTER_ADDR_OFFSET + 1));
    registerValue = (uint16_t)(*(pPdu + FUNC_REGISTER_VAL_OFFSET) << 8);
    registerValue |= (uint16_t)(*(pPdu + FUNC_REGISTER_VAL_OFFSET + 1));

    if (mcWriteRegister(registerAddress, registerValue) != ERROR_OK) {
      excep = MB_EXCEP_ILLVALUE;
    }
  }
  else {
    excep = MB_EXCEP_ILLVALUE;
  }
  return excep;
}
