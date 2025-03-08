#ifndef PC_OSCOMPAT_H
#define PC_OSCOMPAT_H

#include "libultraship/libultraship.h"
#include "bk_bool.h"
#include "enums.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SP_UCODE_SIZE           4096
#define SP_UCODE_DATA_SIZE      2048

#define VI_CTRL_TYPE_16		 0x00002    /* Bit [1:0] pixel size: 16 bit */
#define VI_CTRL_TYPE_32		 0x00003    /* Bit [1:0] pixel size: 32 bit */
#define VI_CTRL_GAMMA_DITHER_ON	 0x00004    /* Bit 2: default = on */
#define VI_CTRL_GAMMA_ON	 0x00008    /* Bit 3: default = on */
#define VI_CTRL_DIVOT_ON	 0x00010    /* Bit 4: default = on */
#define VI_CTRL_SERRATE_ON	 0x00040    /* Bit 6: on if interlaced */
#define VI_CTRL_ANTIALIAS_MASK	 0x00300    /* Bit [9:8] anti-alias mode */
#define VI_CTRL_DITHER_FILTER_ON 0x10000    /* Bit 16: dither-filter mode */

#define AI_STATUS_FIFO_FULL	0x80000000

#define AI_MAX_DAC_RATE         16384           /* 14-bit+1 */
#define AI_MIN_DAC_RATE         132

#define AI_MAX_BIT_RATE         16              /* 4-bit+1 */
#define AI_MIN_BIT_RATE         2


extern s32 osViClock;

#define PI_DOM1_ADDR1		0x06000000	/* to 0x07FFFFFF */
#define PI_DOM1_ADDR2		0x10000000	/* to 0x1FBFFFFF */
#define PI_DOM1_ADDR3		0x1FD00000	/* to 0x7FFFFFFF */
#define PI_DOM2_ADDR1		0x05000000	/* to 0x05FFFFFF */
#define PI_DOM2_ADDR2		0x08000000	/* to 0x0FFFFFFF */

extern OSPiHandle      *__osPiTable; 

#define CHNL_ERR_MASK		0xC0	/* Bit 6-7: channel errors */

#define SET_ACTIVEBANK_TO_ZERO                                                 \
  if (pfs->activebank != 0) {                                                  \
    pfs->activebank = 0;                                                       \
    ERRCK(__osPfsSelectBank(pfs));                                             \
  }


extern OSPifRam __osPfsPifRam;
extern OSPifRam __osContPifRam;
extern u8 __osContLastCmd;

extern u64	osClockRate;

extern u8 __osMaxControllers;

#define CHNL_ERR(format) ((format.rxsize & CHNL_ERR_MASK) >> 4)

#define LEO_BLOCK_MODE	1
#define LEO_TRACK_MODE	2
#define LEO_SECTOR_MODE	3

#define	PI_STATUS_RESET		0x01
#define	PI_SET_RESET		PI_STATUS_RESET

#define	PI_STATUS_CLR_INTR	0x02
#define	PI_CLR_INTR		PI_STATUS_CLR_INTR

#define	PI_STATUS_ERROR		0x04
#define	PI_STATUS_IO_BUSY	0x02
#define	PI_STATUS_DMA_BUSY	0x01

#define OS_PIM_STACKSIZE	4096
#define OS_VIM_STACKSIZE	4096
#define OS_SIM_STACKSIZE	4096

#define	OS_MIN_STACKSIZE	72

extern s32 osPiRawStartDma(s32, u32, void *, u32);
extern s32 osEPiRawStartDma(OSPiHandle *, s32 , u32 , void *, u32 );

extern void 	*osRomBase;

#define	SI_STATUS_DMA_BUSY	0x0001
#define	SI_STATUS_RD_BUSY	0x0002
#define	SI_STATUS_DMA_ERROR	0x0008
#define	SI_STATUS_INTERRUPT	0x1000

#define SP_STATUS_HALT		0x001		/* Bit  0: halt */
#define SP_STATUS_BROKE		0x002		/* Bit  1: broke */
#define SP_STATUS_DMA_BUSY	0x004		/* Bit  2: dma busy */
#define SP_STATUS_DMA_FULL	0x008		/* Bit  3: dma full */
#define SP_STATUS_IO_FULL	0x010		/* Bit  4: io full */
#define SP_STATUS_SSTEP		0x020		/* Bit  5: single step */
#define SP_STATUS_INTR_BREAK	0x040		/* Bit  6: interrupt on break */
#define SP_STATUS_SIG0		0x080		/* Bit  7: signal 0 set */
#define SP_STATUS_SIG1		0x100		/* Bit  8: signal 1 set */
#define SP_STATUS_SIG2		0x200		/* Bit  9: signal 2 set */
#define SP_STATUS_SIG3		0x400		/* Bit 10: signal 3 set */
#define SP_STATUS_SIG4		0x800		/* Bit 11: signal 4 set */
#define SP_STATUS_SIG5	       0x1000		/* Bit 12: signal 5 set */
#define SP_STATUS_SIG6	       0x2000		/* Bit 13: signal 6 set */
#define SP_STATUS_SIG7	       0x4000		/* Bit 14: signal 7 set */

#define SP_CLR_YIELD		SP_CLR_SIG0
#define SP_SET_YIELD		SP_SET_SIG0
#define SP_STATUS_YIELD		SP_STATUS_SIG0
#define SP_CLR_YIELDED		SP_CLR_SIG1
#define SP_SET_YIELDED		SP_SET_SIG1
#define SP_STATUS_YIELDED	SP_STATUS_SIG1
#define SP_CLR_TASKDONE		SP_CLR_SIG2
#define SP_SET_TASKDONE		SP_SET_SIG2
#define SP_STATUS_TASKDONE	SP_STATUS_SIG2
#define	SP_CLR_RSPSIGNAL	SP_CLR_SIG3
#define	SP_SET_RSPSIGNAL	SP_SET_SIG3
#define	SP_STATUS_RSPSIGNAL	SP_STATUS_SIG3
#define	SP_CLR_CPUSIGNAL	SP_CLR_SIG4
#define	SP_SET_CPUSIGNAL	SP_SET_SIG4
#define	SP_STATUS_CPUSIGNAL	SP_STATUS_SIG4

typedef struct {
  /* 0x0 */ u16 state;
  /* 0x2 */ u16 retraceCount;
  /* 0x4 */ void *framep;
  /* 0x8 */ OSViMode *modep;
  /* 0xC */ u32 control;
  /* 0x10 */ OSMesgQueue *msgq;
  /* 0x14 */ OSMesg msg;
  /* 0x18 */ __OSViScale x;
  /* 0x24 */ __OSViScale y;
} __OSViContext;

extern s32 osTvType;	/* 0 = PAL, 1 = NTSC, 2 = MPAL */

#define OS_TV_TYPE_PAL 0
#define OS_TV_TYPE_NTSC 1
#define OS_TV_TYPE_MPAL 2

extern OSViMode	osViModePalLpn1;	/* Individual VI PAL modes */
extern OSViMode	osViModePalLpf1;
extern OSViMode	osViModePalLan1;
extern OSViMode	osViModePalLaf1;
extern OSViMode	osViModePalLpn2;
extern OSViMode	osViModePalLpf2;
extern OSViMode	osViModePalLan2;
extern OSViMode	osViModePalLaf2;
extern OSViMode	osViModePalHpn1;
extern OSViMode	osViModePalHpf1;
extern OSViMode	osViModePalHan1;
extern OSViMode	osViModePalHaf1;
extern OSViMode	osViModePalHpn2;
extern OSViMode	osViModePalHpf2;

extern OSViMode	osViModeMpalLpn1;	/* Individual VI MPAL modes */
extern OSViMode	osViModeMpalLpf1;
extern OSViMode	osViModeMpalLan1;
extern OSViMode	osViModeMpalLaf1;
extern OSViMode	osViModeMpalLpn2;
extern OSViMode	osViModeMpalLpf2;
extern OSViMode	osViModeMpalLan2;
extern OSViMode	osViModeMpalLaf2;
extern OSViMode	osViModeMpalHpn1;
extern OSViMode	osViModeMpalHpf1;
extern OSViMode	osViModeMpalHan1;
extern OSViMode	osViModeMpalHaf1;
extern OSViMode	osViModeMpalHpn2;
extern OSViMode	osViModeMpalHpf2;

extern OSViMode	osViModeTable[];	/* Global VI mode table */

extern OSViMode	osViModeNtscLpn1;	/* Individual VI NTSC modes */
extern OSViMode	osViModeNtscLpf1;
extern OSViMode	osViModeNtscLan1;
extern OSViMode	osViModeNtscLaf1;
extern OSViMode	osViModeNtscLpn2;
extern OSViMode	osViModeNtscLpf2;
extern OSViMode	osViModeNtscLan2;
extern OSViMode	osViModeNtscLaf2;
extern OSViMode	osViModeNtscHpn1;
extern OSViMode	osViModeNtscHpf1;
extern OSViMode	osViModeNtscHan1;
extern OSViMode	osViModeNtscHaf1;
extern OSViMode	osViModeNtscHpn2;
extern OSViMode	osViModeNtscHpf2;

extern __OSViContext *__osViNext;

#define VI_STATE_01 0x01
#define VI_STATE_XSCALE_UPDATED 0x02
#define VI_STATE_YSCALE_UPDATED 0x04
#define VI_STATE_08 0x08         //related to control regs changing
#define VI_STATE_10 0x10         //swap buffer
#define VI_STATE_BLACK 0x20      //probably related to a black screen
#define VI_STATE_REPEATLINE 0x40 //repeat line?
#define VI_STATE_FADE 0x80       //fade

extern __OSViContext *__osViCurr;
__OSViContext *__osViGetCurrentContext(void);

#define VI_CTRL_ANTIALIAS_MODE_3 0x00300 /* Bit [9:8] anti-alias mode */
#define VI_CTRL_ANTIALIAS_MODE_2 0x00200 /* Bit [9:8] anti-alias mode */
#define VI_CTRL_ANTIALIAS_MODE_1 0x00100 /* Bit [9:8] anti-alias mode */

#define BURST(hsync_width, color_width, vsync_width, color_start) \
    (hsync_width | (color_width << 8) | (vsync_width << 16) | (color_start << 20))
#define WIDTH(v) v
#define VSYNC(v) v
#define HSYNC(duration, leap) (duration | (leap << 16))
#define LEAP(upper, lower) ((upper << 16) | lower)
#define START(start, end) ((start << 16) | end)
#define VCURRENT(v) v //seemingly unused
#define ORIGIN(v) v
#define VINTR(v) v
#define HSTART START

// Updated SCALE macro to use integer arithmetic
#define SCALE(scaleup, off) (((int)(1.0f / (scaleup) * (1 << 10))) | ((int)(off) << 16))

#define VI_SCALE_MASK 0xfff //see rcp scale_x/scale_y
#define VI_2_10_FPART_MASK 0x3ff
#define VI_SUBPIXEL_SH 0x10

#define PIF_ROM_START		0x1FC00000
#define PIF_ROM_END		0x1FC007BF
#define PIF_RAM_START		0x1FC007C0
#define PIF_RAM_END		0x1FC007FF

extern s32 	osResetType;	/* 0 = cold reset, 1 = NMI */
extern s32	osAppNMIBuffer[];

#define OS_APP_NMI_BUFSIZE	64

extern OSTimer __osBaseTimer;

extern struct __osThreadTail
{
    OSThread *next;
    OSPri priority;
} __osThreadTail;

extern OSThread *__osActiveQueue;

#define LEO_CMD_TYPE_0 0 //TODO: name
#define LEO_CMD_TYPE_1 1 //TODO: name
#define LEO_CMD_TYPE_2 2 //TODO: name
#define LEO_BM_CTL (LEO_BASE_REG + 0x510)
#define LEO_ERROR_29 29 //
#define LEO_BM_CTL_RESET 0x10000000 
#define LEO_STATUS (LEO_BASE_REG + 0x508)

void osInitialize(void);
s32 mapSpecificFlags_validateCRC1(void);
void osCreateThread(OSThread *, OSId, void (*)(void *), void *, void *,
                           OSPri);
#define bzero(dst, size) memset(dst, 0, size)
s32 osAiSetFrequency(u32 frequency);
void osStopThread(OSThread *t);
void osStartThread(OSThread *t);
void osSpTaskLoad(OSTask *intp);
void osSpTaskStartGo(OSTask *tp);
u32 osDpGetStatus();
void osSpTaskYield(void);
u32	__osGetSR(void);
u32 func_8025C29C(u32 *seed);
void osDestroyThread(OSThread *t);
void osSetThreadPri(OSThread *t, OSPri pri);
u32 ___osGetSR(void);
s32 osContSetCh(u8 ch);
void viMgr_clearFramebuffers(void);
void func_80253010(void *dest, void *src, s32 size);
void func_80253034(void *framebuffers, int value, int size);
void overlayManagerloadCore2(void);
int overlayManagergetLoadedId(void);

bool overlayManagerload(enum overlay_e overlay_id);

void guTranslate(Mtx *m, float x, float y, float z);
void guOrtho(Mtx *m, float l, float r, float b, float t, float n, float f, float scale);
void guRotate(Mtx *m, float a, float x, float y, float z);
void guMtxIdentF(float mf[4][4]);
void guMtxF2L(float mf[4][4], Mtx *m);
float gu_sqrtf(float val);

extern u8 D_8000E800;
extern int defaultHuft;
extern u8 n_aspMainTextStart[];
extern u8 n_aspMainTextEnd[];
extern u8 gSPF3DEX_fifoTextStart[];
extern u8 gSPF3DEX_fifoTextEnd[];
extern u8 gSPF3DEX_fifoDataStart[];
extern u8 gSPL3DEX_fifoDataStart[];
extern u8 gSPF3DEX_fifoDataEnd[];
extern u8 gSPL3DEX_fifoTextStart[];
extern u8 gSPL3DEX_fifoTextEnd[];
extern s32 osCicId;
extern u8 n_aspMainDataStart[];
extern u8 n_aspMainDataEnd[];

#ifdef __cplusplus
}
#endif


#endif // PC_OSCOMPAT_H