#ifndef CRC_H_INCLUDED
#define CRC_H_INCLUDED

#define CRC0000                                         //Uncomment to get constant 0x0000 crc in debug purposes

uint16_t getCrc16(uint8_t *data, uint16_t length);      //Get CRC16 for *data with length

#endif // CRC_H_INCLUDED
