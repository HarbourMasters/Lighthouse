#include <ultra64.h>
#include "rarezip.h"

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#endif

#define ENTRY_STACK_LEN 0x2000
#define ENTRY_STACK_LEN_U64 (ENTRY_STACK_LEN / sizeof(u64))

u64 gEntryStack[ENTRY_STACK_LEN_U64];

extern u8 core1_rzip_ROM_START[];
extern u8 core1_rzip_ROM_END[];
extern u8 heapBlocks;
extern u8 core1_VRAM;
extern u32 a_gRomCrcValues[4];

void func_80000594(u8 **, u8 **);
void func_8023DA20(s32);

void func_80000450(s32 arg0){
    u8 *tmp;
    u8 *dst;
    
    tmp = &heapBlocks;
    dst = &core1_VRAM;
    osInitialize();
    osPiRawStartDma(OS_READ, core1_rzip_ROM_START, tmp, core1_rzip_ROM_END - core1_rzip_ROM_START);
    while(osPiGetStatus() & PI_STATUS_DMA_BUSY);
    func_8000055C();
    func_80000594(&tmp, &dst);
    a_gRomCrcValues[0] = crc1;
    a_gRomCrcValues[1] = crc2;
    func_80000594(&tmp, &dst);
    a_gRomCrcValues[2] = crc1;
    a_gRomCrcValues[3] = crc2;
    overlay_table_init();
    (&func_8023DA20)(arg0);
    
}
