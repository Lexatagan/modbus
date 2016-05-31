#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#define PDU_SIZE_MIN                1
#define PDU_SIZE_MAX                253
#define PDU_FUNC_OFFSET             0
#define PDU_DATA_OFFSET             1

#define FUNC_WRITE_REGISTER_SIZE    0x04
#define FUNC_READ_REGISTERS_SIZE    0x04
#define FUNC_READ_REGISTERS_MAX     0x7D
#define FUNC_REGISTER_ADDR_OFFSET   PDU_DATA_OFFSET
#define FUNC_REGISTER_ADDR_SIZE     2
#define FUNC_REGISTER_VAL_OFFSET    PDU_DATA_OFFSET + FUNC_REGISTER_ADDR_SIZE
#define FUNC_REGISTER_VAL_SIZE      2

MB_ExceptionTypeDef APP_WriteHoldingRegister (uint8_t *pPdu, uint16_t *pPduLength);         //Function #06
MB_ExceptionTypeDef APP_ReadHoldingRegisters(uint8_t *pPdu, uint16_t *pPduLength);          //Function #03


#endif // APP_H_INCLUDED
