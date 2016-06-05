#ifndef MB_H_INCLUDED
#define MB_H_INCLUDED


#define MODBUS_BIG_BAUDRATE         19200         //Fixed intervals used on big baud rates
#define TCHAR                       500           //us char length, used in calculations at big baud rates

#define MB_ADDR_BROADCAST           0             //ModBus broadcast address
#define MB_ADDR_MIN                 1             //ModBus slave min address
#define MB_ADDR_MAX                 247           //ModBus slave max address

#define MB_SIZE_MAX                 256           //Serial PDU frame max size
#define MB_SIZE_MIN                 4             //Serial PDU frame min size (addr + func + crc16)
#define MB_ADDR_OFFSET              0             //Address offset
#define MB_ADDR_LENGTH              1             //Address length
#define MB_CRC_LENGTH               2             //CRC16 length

#define MB_FUNC_WRITE_REGISTER      0x06          //Used functions
#define MB_FUNC_READ_REGISTERS      0x03
#define MB_FUNC_ERROR               0x80

typedef enum {                                    //ModBus state
  MB_STATE_ENABLED,
  MB_STATE_DISABLED,
  MB_STATE_NOT_INITIALIZED
} MB_StateTypeDef;

typedef enum {                                    //ModBus errors
  MB_ERROR_OK,
  MB_ERROR_ILLSTATE,
  MB_ERROR_ILLREG,
  MB_ERROR_ILLVAL,
  MB_ERROR_IOFAIL,
  MB_ERROR_TIMEOUT
} MB_ErrorTypeDef;

typedef enum {                                    //UART receiver state
  RX_STATE_INIT,
  RX_STATE_IDLE,
  RX_STATE_RCV,
  RX_STATE_T35EXPECTED
} MB_RcvStateTypeDef;

typedef enum {                                    //UART transceiver state
  TX_STATE_IDLE,
  TX_STATE_TMIT
} MB_TsvStateTypeDef;

typedef enum {                                    //ModBus events
  MB_EVENT_NONE,
  MB_EVENT_ERROR,
  MB_EVENT_READY,
  MB_EVENT_FRAME_RECEIVED,
  MB_EVENT_EXECUTE,
  MB_EVENT_SENT
} MB_EventTypeDef;

typedef enum {                                    //ModBus exceptions
  MB_EXCEP_NONE = 0x00,
  MB_EXCEP_ILLFUNCTION = 0x01,
  MB_EXCEP_ILLVALUE = 0x03,
} MB_ExceptionTypeDef;

MB_ErrorTypeDef MB_init(uint8_t devAddr, uint32_t baudRate, uint32_t cpuFrequency); //Initialize ModBus
MB_ErrorTypeDef MB_start();                                                         //Start ModBus
MB_ErrorTypeDef MB_stop();                                                          //Stop ModBus
MB_ErrorTypeDef MB_poll();                                                          //Should be called periodically to process received frames

#endif // MB_H_INCLUDED
