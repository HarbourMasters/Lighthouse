#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include <PR/rcp.h>
#include <ultra64.h>
#endif

#define UCODE_SIZE 256

static u8 ucodeBuffer[UCODE_SIZE];
static s32 ucodeCheckValue1;
static s32 ucodeCheckValue2;
static s32 ucodeStatus;

void ucode_load(void) {
  ucodeCheckValue2 = *(s32 *)PHYS_TO_K1(0x04000000) ^ -1;
  ucodeStatus = ucodeCheckValue2 ? 0x01 : 0x00;

  ucodeCheckValue1 = *(s32 *)PHYS_TO_K1(0x04001000) ^ 0x17D7;
  ucodeStatus |= ucodeCheckValue1 ? 0x02 : 0x00;

  if (ucodeStatus == 0) {
    piMgr_read(&ucodeBuffer, 0xB0000B70, UCODE_SIZE);
  }
}

void ucode_stub1(void) {}

void ucode_stub2(void) { osPiReadIo(0, NULL); }

s32 ucode_stub3(void) { return 0; }

void ucode_getPtrAndSize(void **ptr, u32 *size) {
  *ptr = &ucodeBuffer;
  *size = UCODE_SIZE;
}
