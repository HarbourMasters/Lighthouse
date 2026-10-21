#ifndef PORT_ENHANCEMENTS_COSMETICS_PLAYERCOLORS_H
#define PORT_ENHANCEMENTS_COSMETICS_PLAYERCOLORS_H

#include <ultra64.h>

#include "port/UI/cvar_prefixes.h"

#define CVAR_PLAYER_PACK_TEXTURES CVAR_COSMETIC("Player.PackTextures")

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BK_COLOR_BANJO_FUR = 0,
    BK_COLOR_BANJO_SHORTS,
    BK_COLOR_KAZOOIE_FEATHERS,
    BK_COLOR_KAZOOIE_BEAK,
    BK_COLOR_BANJO_BACKPACK,
    BK_COLOR_BANJO_SKIN,
    BK_COLOR_CHANNEL_COUNT
} BKColorChannel;

typedef struct {
    u8 enabled;
    u8 r, g, b;
} BKColorChannelSetting;

typedef struct {
    BKColorChannelSetting channel[BK_COLOR_CHANNEL_COUNT];
} BKPlayerColorSet;

#define BK_COLORS_LOCAL_OWNER 0u

void PlayerColors_getVanilla(s32 channel, u8* red, u8* green, u8* blue);
const char* PlayerColors_getChannelCVar(s32 channel);
const char* PlayerColors_getChannelEnabledCVar(s32 channel);
void PlayerColors_applyForDraw(u32 ownerKey, s32 modelId, void* modelBin, const BKPlayerColorSet* colors);
void PlayerColors_getLocal(BKPlayerColorSet* out);
s32 PlayerColors_hasPackTextures(void);
void PlayerColors_forgetOwner(u32 ownerKey);
void PlayerColors_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* PORT_ENHANCEMENTS_COSMETICS_PLAYERCOLORS_H */
