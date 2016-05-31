#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "modbus_library/mb.h"
#include "unitTests.h"
#include "bsp.h"

int main()
{
  startUnitTests();
  if (MB_init(MB_SLAVE_ADDR, 38400, CPU_FREQ) == MB_ERROR_OK)
    MB_start();
  while(true) {
    MB_poll();
  }
  return 0;
}
