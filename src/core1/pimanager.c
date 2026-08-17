// BanjoDecomp: pimanager.c
#include <ultra64.h>
#include "core1/core1.h"
#include "version.h"

#define PIMANAGER_MESGBUFFER_SIZE 16

#if VERSION == VERSION_USA_1_0
#define BLOCK_SIZE 0x20000
#elif VERSION == VERSION_PAL
#define BLOCK_SIZE 0x8000
#endif

struct {
    OSIoMesg io_mesg;
    OSMesg mesg;
    OSMesgQueue queue;
} sPiManagerDMAData;

struct {
    OSMesg mesg[PIMANAGER_MESGBUFFER_SIZE];
    OSMesgQueue queue;
} sPiManagerData;

void parallel_readDMA(void *vaddr, s32 devaddr, s32 size) {
    s32 block_cnt;
    s32 block_remainder;
    int i;

    osWritebackDCache(vaddr, size);
    block_cnt       = size / BLOCK_SIZE;
    block_remainder = size % BLOCK_SIZE;

    for (i = 0; i < block_cnt; i++) {
        osPiStartDma(&sPiManagerDMAData.io_mesg, OS_MESG_PRI_NORMAL, OS_READ, devaddr, vaddr, BLOCK_SIZE, &sPiManagerDMAData.queue);
        osRecvMesg(&sPiManagerDMAData.queue, NULL, OS_MESG_BLOCK);
        devaddr += BLOCK_SIZE;
        vaddr = (u8 *) vaddr + BLOCK_SIZE;
    }

    osPiStartDma(&sPiManagerDMAData.io_mesg, OS_MESG_PRI_NORMAL, OS_READ, devaddr, vaddr, block_remainder, &sPiManagerDMAData.queue);
    osRecvMesg(&sPiManagerDMAData.queue, NULL, OS_MESG_BLOCK);
    osInvalDCache(vaddr, size);
}

void parallel_init(void) {
    osCreateMesgQueue(&sPiManagerDMAData.queue, &sPiManagerDMAData.mesg, 1);
    osCreateMesgQueue(&sPiManagerData.queue, sPiManagerData.mesg, PIMANAGER_MESGBUFFER_SIZE);
    osCreatePiManager(OS_PRIORITY_PIMGR, &sPiManagerData.queue, sPiManagerData.mesg, PIMANAGER_MESGBUFFER_SIZE);
}
