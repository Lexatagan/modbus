#ifndef BSP_H_INCLUDED
#define BSP_H_INCLUDED

#define CPU_FREQ            1000000                         //CPU clock frequency

#define MB_SLAVE_ADDR       0x30                            //This device ModBus address

#define MC_BASE             memCard                         //Memory card start address
#define MC_SIZE             0x40                            //Memory card total size
#define MC_ROREGION_START   0x00                            //Read only region start address
#define MC_ROREGION_END     0x39                            //Read only region end address
#define MC_RWREGION_START   0x21                            //Read/Write region start address
#define MC_RWREGION_END     0x39                            //Read/Write region end address

#define UART_BASE           uartReg                         //UART registers start address

#define TIMER_BASE          timerReg                        //Timer registers start address

extern uint16_t memCard[];
extern __IO uint32_t uartReg[];
extern __IO uint32_t timerReg[];

#endif // BSP_H_INCLUDED
