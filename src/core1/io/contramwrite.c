#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else

#include "controller.h"
#include "siint.h"
#include <os_internal.h>
#include <rcp.h>
#endif

static void __osPackRamWriteData(int channel, u16 address, u8 *buffer);
s32 __osContRamWrite(OSMesgQueue *mq, int channel, u16 address, u8 *buffer,
                     int force) {
  s32 ret;
  int i;
  u8 *ptr;
  __OSContRamHeader ramreadformat;
  int retry;

  ret = 0;
  ptr = (u8 *)&__osPfsPifRam;
  retry = 2;
  if (force != 1 && address < 7 && address != 0)
    return 0;
  __osSiGetAccess();
  __osContLastCmd = CONT_CMD_WRITE_MEMPACK;
  __osPackRamWriteData(channel, address, buffer);
  ret = __osSiRawStartDma(OS_WRITE, &__osPfsPifRam);
  osRecvMesg(mq, NULL, OS_MESG_BLOCK);
  do {
    ret = __osSiRawStartDma(OS_READ, &__osPfsPifRam);
    osRecvMesg(mq, NULL, OS_MESG_BLOCK);
    ptr = (u8 *)&__osPfsPifRam;
    if (channel != 0)
      for (i = 0; i < channel; i++)
        ptr++;

    ramreadformat = *(__OSContRamHeader *)ptr;

    ret = CHNL_ERR(ramreadformat);
    if (ret == 0) {
      if (__osContDataCrc(buffer) != ramreadformat.datacrc) {
        ret = __osPfsGetStatus(mq, channel);
        if (ret != 0) {
          __osSiRelAccess();
          return ret;
        }
        ret = PFS_ERR_CONTRFAIL;
      }
    } else {
      ret = PFS_ERR_NOPACK;
    }
    if (ret != PFS_ERR_CONTRFAIL)
      break;
  } while ((retry-- >= 0));
  __osSiRelAccess();
  return ret;
}

static void __osPackRamWriteData(int channel, u16 address, u8 *buffer) {
  u8 *ptr;
  __OSContRamHeader ramreadformat;
  int i;

  ptr = (u8 *)__osPfsPifRam.ram;

  for (i = 0; i < ARRLEN(__osPfsPifRam.ram) + 1;
       i++) { // also clear status
    __osPfsPifRam.ram[i] = 0;
  }

  __osPfsPifRam.status = CONT_CMD_EXE;
  ramreadformat.unk_00 = CONT_CMD_NOP;
  ramreadformat.txsize = CONT_CMD_WRITE_MEMPACK_TX;
  ramreadformat.rxsize = CONT_CMD_WRITE_MEMPACK_RX;
  ramreadformat.poll = CONT_CMD_WRITE_MEMPACK;
  ramreadformat.hi = (address << 0x5) | __osContAddressCrc(address);
  ramreadformat.datacrc = CONT_CMD_NOP;
  for (i = 0; i < ARRLEN(ramreadformat.data); i++) {
    ramreadformat.data[i] = *buffer++;
  }
  if (channel != 0) {
    for (i = 0; i < channel; i++) {
      *ptr++ = 0;
    }
  }
  *(__OSContRamHeader *)ptr = ramreadformat;
  ptr += sizeof(__OSContRamHeader);
  ptr[0] = CONT_CMD_END;
}
