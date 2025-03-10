#include <ultra64.h>
#include "functions.h"
#include "variables.h"

int __osSiDeviceBusy()
{
    #ifndef LIGHTHOUSE_P
    register u32 stat = IO_READ(SI_STATUS_REG);
    if (stat & (SI_STATUS_DMA_BUSY | SI_STATUS_RD_BUSY))
        return 1;
    return 0;
    #else
    return 0;
    #endif
}
