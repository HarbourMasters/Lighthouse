// BanjoDecomp: core2/code_B8020.c
#include <ultra64.h>
#include "structs.h"

BKSpriteTextureBlock *sNullSpriteTextureBlock;

BKSpriteTextureBlock *spritemask_getChunk(BKSpriteMask *list, s32 index) {
    s32 *offset_ptr = &list->offset[index];
    BKSpriteTextureBlock *chunk;

    if (*offset_ptr == -1) {
        return sNullSpriteTextureBlock;
    }

    chunk = (BKSpriteTextureBlock *) (*offset_ptr + (u8 *) &((*list).offset[list->count]));
    return chunk;
}
