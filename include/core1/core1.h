#ifndef BANJO_KAZOOIE_CORE1_CORE1_H
#define BANJO_KAZOOIE_CORE1_CORE1_H

#include <ultra64.h>
#include <PR/libaudio.h> // [port] for ALSeqpConfig/ALHeap in audioManager decls (decomp gets this via SDK ultra64.h)
#include "bool.h"
#include "enums.h"
#include "structs.h"
#include "prop.h"

#include "core1/audiomanager.h"
#include "core1/bamotor.h"
#include "core1/debugtext.h"
#include "core1/defragthread.h"
#include "core1/depthbuffer.h"
#include "core1/eeprom.h"
#include "core1/framebufferdraw.h"
#include "core1/glcrc.h"
#include "core1/initthread.h"
#include "core1/lookup.h"
#include "core1/main.h"
#include "core1/mem.h"
#include "core1/ml.h"
#include "core1/mlmtx.h"
#include "core1/midichannel.h"
#include "core1/music.h"
#include "core1/musicplayer.h"
#include "core1/overlay.h"
#include "core1/overlaymanager.h"
#include "core1/parallel.h"
#include "core1/pfsmanager.h"
#include "core1/rarezip.h"
#include "core1/sns.h"
#include "core1/sprite.h"
#include "core1/stub_1D590.h"
#include "core1/thread5.h"
#include "core1/ucode.h"
#include "core1/viewport.h"
#include "core1/vimgr.h"

#ifdef __cplusplus
extern "C" {
#endif


/* need to sort out in individual header files */
void* malloc(size_t size);

void func_8025A104(enum comusic_e arg0, s32 arg1);
void func_8025A55C(s32 arg0, s32 arg1, s32 arg2);
s32 func_8025A864(enum comusic_e track_id);
void func_8025ABB8(enum comusic_e track_id, s32 arg1, s32 arg2, s32 arg3);


/* src/core1/code_7090.c */

void core1_7090_alloc(void);
void core1_7090_release(void);
void core1_7090_initSfxSource(s32 idx, s32 lookup_idx, s32 sample_rate, f32 volume);
void core1_7090_freeSfxSource(int idx);


/* src/core1/code_15B30.c */

#define UCODE_TASK_TYPE_AUDIO 0
#define UCODE_TASK_TYPE_F3DEX 1
#define UCODE_TASK_TYPE_L3DEX 2
#define UCODE_TASK_TYPE_FRAMEBUFFER_CHANGED 7

struct ucode_task_data_s {
    s32 task_type; // 0 - audio task, 1 - f3dex task, 2 - l3dex task, 7 - probably to signal framebuffers swapped
    s32 unk4; // is only set for gfx tasks (0 or 0x40000000)
    void *data_ptr; // begin of dlist data (was u64* upstream; void* here for port-side casts)
    void *data_ptr_end; // end of dlist data
    OSMesgQueue *audio_mesg_queue; // only relevant for audio tasks
    OSMesg audio_mesg; // only relevant for audio tasks
};

#define DEFAULT_FRAMEBUFFER_WIDTH 292
#define DEFAULT_FRAMEBUFFER_HEIGHT 216

extern s32 gFramebufferWidth;
extern s32 gFramebufferHeight;
extern u16 gFramebuffers[2][DEFAULT_FRAMEBUFFER_WIDTH * DEFAULT_FRAMEBUFFER_HEIGHT];

void core1_15B30_requestLockForTaskDataID(void);
void core1_15B30_requestReleaseForTaskDataID(void);
void core1_15B30_addAudioTaskData(Acmd *start, Acmd *end, OSMesgQueue *mesg_queue, OSMesg msg);
void setupFramebuffer(Gfx **gfx, void *color_buffer);
void setupFramebufferForGamemode(Gfx **gfx, s32 framebuffer_idx);
void setupScissorBoxAndFramebuffer(Gfx **gfx, void *color_buffer);
void setupDefaultScissorBoxAndFramebuffer(Gfx **gfx, s32 framebuffer_idx);
void core1_15B30_finishDList_renderThread(Gfx **gfx);
void core1_15B30_finishDList(Gfx **gfx);
void core1_15B30_addF3DEXTaskData(Gfx *start, Gfx *end, s32 flags);
void core1_15B30_addF3DEXTaskData_0(Gfx *start, Gfx *end);
void core1_15B30_addF3DEXTaskData_40000000(Gfx *start, Gfx *end);
void core1_15B30_addL3DEXTaskData(Gfx **start, Gfx **end, s32 flags);
void core1_15B30_addL3DEXTaskData_0(Gfx **start, Gfx **end);
void core1_15B30_addL3DEXTaskData_40000000(Gfx **start, Gfx **end);
void scissorBox_get(u32 *left, u32 *top, u32 *right, u32 *bottom);
void func_80253FE8(void);
void core1_15B30_sendMesg3ToRenderThread(void);
void core1_15B30_init(void);
void drawRectangle2D(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b);
void graphicsCache_release(void);
void graphicsCache_init(void);
void graphicsCache_checkFrame(Gfx *gfxStart, Gfx *gfxEnd, Mtx *mtxStart, Mtx *mtxEnd, Vtx *vtxStart, Vtx *vtxEnd);
void scissorBox_set(s32 left, s32 top, s32 right, s32 bottom);
void scissorBox_setDefault(void);
void core1_15B30_addTask7TaskData(s32 framebuffer_id);
void core1_15B30_toggleTexturePointFilter(void);
void graphicscache_swapAndGetStacks(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void dummy_func_80254464(void);

#ifdef __cplusplus
}
#endif

#endif
