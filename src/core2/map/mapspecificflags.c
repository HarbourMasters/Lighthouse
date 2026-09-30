// BanjoDecomp: core2/mapspecificflags.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#define CHECKSUM_SALT_A             0x01195E97
#define CHECKSUM_SALT_B             0xA84E38C8
#define CHECKSUM_SALT_C             0x3973E4D9

/* .data */
u32 D_80367000 = 0;

/* .bss */
u32 D_8037DDE0;
u32 D_8037DDE4;
u32 D_8037DDE8;

/* .code */
u32 mapSpecificFlags_calculateChecksum(void) {
    return D_80367000 ^ CHECKSUM_SALT_A;
}

void mapSpecificFlags_updateCRCs(void) {
    D_8037DDE4 = D_80367000 ^ CHECKSUM_SALT_B;
    D_8037DDE0 = mapSpecificFlags_calculateChecksum();
    D_8037DDE8 = D_80367000 ^ CHECKSUM_SALT_C;
}

void mapSpecificFlags_clearAll(void) {
    D_80367000 = 0;
    mapSpecificFlags_updateCRCs();
}

bool mapSpecificFlags_get(s32 index) {
    return BOOL(D_80367000 & (1 << index));
}

u32 mapSpecificFlags_getN(s32 start_index, s32 count) {
    int i;
    u32 ret_val = 0;

    for (i = 0; i < count; i++) {
       ret_val |= mapSpecificFlags_get(start_index + i) << i;
    }

    return ret_val;
}

bool mapSpecificFlags_getAndReset(s32 index) {
    bool ret_val = mapSpecificFlags_get(index);
//  mapSpecificFlags_set(index, 0);
    mapSpecificFlags_setEx(index, 0, 0);
    return ret_val;
}

void mapSpecificFlags_setEx(s32 index, s32 value, s32 triggerEvent) {
    s32 changed = ((D_80367000 >> index) & 1) != (value ? 1 : 0);

    if (value) {
        D_80367000 |= 1 << index;
    } else {
        D_80367000 &= ~(1 << index);
    }

    mapSpecificFlags_updateCRCs();

    if (triggerEvent && changed) {
        CALL_EVENT(OnGameFlagSet, ANCHOR_FLAGSPACE_MAP_SPECIFIC, index, value ? 1 : 0, 1);
    }
}

void mapSpecificFlags_set(s32 index, bool value) {
    mapSpecificFlags_setEx(index, value, 1);
}

void mapSpecificFlags_setN(s32 start_index, s32 values, s32 count) {
    int i;

    for (i = 0; i < count; i++) {
       mapSpecificFlags_set(start_index + i, (1 << i) & values);
    }
}

u32 mapSpecificFlags_getAll(void) {
    return D_80367000;
}

void mapSpecificFlags_setAll(u32 flags) {
    D_80367000 = flags;
    mapSpecificFlags_updateCRCs();
}

// Unclear, why the bitfield functions are in this file, while they are only used in gccube.c?

struct bitfield_s *bitfield_new(s32 count) {
    struct bitfield_s *bitfield = (struct bitfield_s*) bk_malloc(sizeof(struct bitfield_s) + ((count + 31) >> 5) * sizeof(s32));
    bitfield->count = count;
    return bitfield;
}

void bitfield_free(struct bitfield_s *this) {
    bk_free(this);
}

void bitfield_setBit(struct bitfield_s *this, s32 index, bool value) {
    if (value) {
        this->data[index >> 5] |= 1 << (index & 0x1F);
    }
    else {
        this->data[index >> 5] -= this->data[index >> 5] & (1 << (index & 0x1F));
    }
}

bool bitfield_getBit(struct bitfield_s *this, s32 index) {
    return BOOL(this->data[index >> 5] & (1 << (index & 0x1F)));
}

void bitfield_setAll(struct bitfield_s *this, bool value) {
    s32 i;

    for (i = 0; i < this->count; i++) {
        bitfield_setBit(this, i, value);
    }
}

#if ANTI_TAMPER
bool mapSpecificFlags_validateCRC1(void) {
    return mapSpecificFlags_calculateChecksum() == D_8037DDE0;
}
#endif
