#ifndef BANJO_KAZOOIE_CORE1_CORE1_H
#define BANJO_KAZOOIE_CORE1_CORE1_H

#include <ultra64.h>

#include "bk_bool.h"
#include "enums.h"
#include "structs.h"
#include "prop.h"

#include "core1/eeprom.h"
#include "core1/framebufferdraw.h"
#include "core1/lookup.h"
#include "core1/main.h"
#include "core1/mem.h"
#include "core1/ml.h"
#include "core1/mlmtx.h"
#include "core1/pfsmanager.h"
#include "core1/rarezip.h"
#include "core1/sns.h"
#include "core1/sprite.h"
#include "core1/ucode.h"
#include "core1/viewport.h"
#include "core1/vimgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* need to sort out in individual header files */

void glcrc_calc_checksum(void *start, void *end, u32 checksum[2]);

void musicTrack_setChannelMask(s32 arg0, u16 chan_mask, f32 arg2);

void comusic_setMainTrack(enum comusic_e arg0, s32 arg1);
void comusic_updateTrackWithArgs(s32 arg0, s32 arg1, s32 arg2);
s32 comusic_stopTrackIfDone(enum comusic_e track_id);
void comusic_fadeTrack(enum comusic_e comusic_id, s32 arg1, s32 arg2, s32 arg3);
int comusic_isTrackQueued(enum comusic_e arg0);
int comusic_isMainTrack(enum comusic_e arg0);


/* src/core1/code_7090.c */

void sfx_alloc(void);
void sfx_release(void);
void sfx_initSfxSource(s32 idx, s32 lookup_idx, s32 sample_rate, f32 volume);
void sfx_freeSfxSource(int idx);


/* src/core1/code_CE60.c */

void map_setChanMask(s32 chan_mask);
void map_setChanMaskWithValue(s32 chan_mask, f32 arg1);
bool map_isPlayerInRange(s32 x, s32 z, s32 distance);
f32 map_getPlayerDistance(f32 x, f32 z);
bool map_isPlayerInsideBoundingBox(s32 box_idx);
void map_updateMusicTracks(s32 arg0);
void map_updateTrackVolumes(void);
void map_updateMusicState(bool arg0);
void map_resetMusicState(void);
void map_resetState(void);
void map_setChanMaskFromWaterState(s32 chan_mask_underwater, s32 chan_mask_surface);
void map_handleMusicEvents(void);
void map_worthlessCounter(bool increment);
void map_setMusicFlag(s32 arg0, s32 arg1);



/* src/core1/depthbuffer.c */

extern u8 D_8000E800;

void depthBuffer_clear(Gfx **gfx);
void depthBuffer_draw(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, void *color_buffer);
bool depthBuffer_isPointerSet(void);
void depthBuffer_stub(void);
void depthBuffer_init(int arg0);
void depthBuffer_setStatus(int arg0);
void depthBuffer_set(Gfx **gfx);
void *depthBuffer_get(void);


/* src/core1/code_15B30.c */

typedef struct {
    s32 unk0;
    s32 unk4;
    Gfx *unk8;
    Gfx *unkC;
    s32 unk10;
    s32 unk14;
}Struct_Core1_15B30;

#define DEFAULT_FRAMEBUFFER_WIDTH 292
#define DEFAULT_FRAMEBUFFER_HEIGHT 216

extern s32 gFramebufferWidth;
extern s32 gFramebufferHeight;
extern u16 gFramebuffers[2][DEFAULT_FRAMEBUFFER_WIDTH * DEFAULT_FRAMEBUFFER_HEIGHT];

void waitForGfxTask(void);
void enqueueGfxTask(Gfx **arg0, Gfx **arg1, UNK_TYPE(s32) arg2, UNK_TYPE(s32) arg3);
void setupGfxTask(Gfx ** gdl, void *arg1);
void scissorBox_SetForGameMode(Gfx **gdl, s32 framebuffer_idx);
void setupScissorBoxAndFramebuffer(Gfx **gfx, s32 framebuffer_address);
void setupDefaultScissorBoxAndFramebuffer(Gfx **gfx, s32 framebuffer_idx);
void endGfxTask(Gfx **gfx);
void finishFrame(Gfx **gdl);
void enqueueGfxTaskWithArg(Gfx *arg0, Gfx *arg1, s32 arg2);
void enqueueGfxTaskNoArg(Gfx *arg0, Gfx *arg1);
void enqueueGfxTaskWithFlag(Gfx *arg0, Gfx *arg1);
void enqueueGfxTaskWithArgs(Gfx **arg0, Gfx **arg1, s32 arg2);
void enqueueGfxTaskNoArgs(Gfx **arg0, Gfx **arg1);
void enqueueGfxTaskWithFlags(Gfx **arg0, Gfx **arg1);
void scissorBox_get(u32 *left, u32 *top, u32 *right, u32 *bottom);
void sendViMessage(void);
void sendGfxTaskMessage(void);
void initGfxTaskQueue(void);
void drawRectangle2D(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b);
void graphicsCache_release(void);
void graphicsCache_init(void);
void scissorBox_set(s32 left, s32 top, s32 right, s32 bottom);
void scissorBox_setDefault(void);
void setActiveFramebuffer(s32 arg0);
void toggleTextureFilterPoint(void);
void getGraphicsStacks(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void dummy_func_80254464(void);
void initThread_create(void);

#ifdef __cplusplus
}
#endif

#endif
