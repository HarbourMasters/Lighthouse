#include <ultra64.h>
#include "functions.h"
#include "variables.h"


//textureList_getDataPtr
u8 *textureList_getDataPtr(BKTextureList *this){
    return (u8*)this + this->count*sizeof(BKTextureInfo) + sizeof(BKTextureList);
}

s32 textureInfo_getBitDepth(BKTextureInfo *this){
    if(this->type & 1){
        return 4;
    }

    if(this->type & 2){
        return 8;
    }

    if(this->type & 4){
        return 0x10;
    }

    if(this->type & 8){
        return 0x20;
    }
    return 0;
}

s32 textureInfo_getType(BKTextureInfo *this){
    return this->type;
}

s32 textureInfo_getPaletteSize(BKTextureInfo *this){
    if(this->type & 1){
        return 0x20;
    }

    if(this->type & 2){
        return 0x200;
    }

    if(this->type & 4){
        return 0;
    }

    if(this->type & 8){
        return 0;
    }
    return 0;
}

// texture_getOffset
s32 func_802EA6DC(BKTextureInfo *this){
    return this->offset;
}

//texture_getSize
s32 func_802EA6E4(BKTextureInfo *this){
    s32 palette_size = textureInfo_getPaletteSize(this);
    s32 pixel_size = textureInfo_getBitDepth(this);

    return (s32)pixel_size*this->width*this->height/8  + palette_size;
}

//textureList_getTexture
BKTextureInfo *func_802EA748(BKTextureList *arg0, s32 indx){
    return (BKTextureInfo *)(arg0 +1) + indx;
}
