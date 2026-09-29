#ifndef BANJO_KAZOOIE_CORE2_MAPSPECIFICFLAGS_H
#define BANJO_KAZOOIE_CORE2_MAPSPECIFICFLAGS_H

#include <ultra64.h>
#include "bool.h"
#include "enums.h"

u32 mapSpecificFlags_calculateChecksum(void);
void mapSpecificFlags_updateCRCs(void);
void mapSpecificFlags_clearAll(void);
bool mapSpecificFlags_get(s32 index);
u32 mapSpecificFlags_getN(s32 start_index, s32 count);
bool mapSpecificFlags_getAndReset(s32 index);
void mapSpecificFlags_set(s32 index, bool value);
void mapSpecificFlags_setN(s32 start_index, s32 values, s32 count);
u32 mapSpecificFlags_getAll(void);
void mapSpecificFlags_setAll(u32 flags);
bool mapSpecificFlags_validateCRC1(void);

// This is like a std::vector<bool> from C++
struct bitfield_s {
    s32 count;
    s32 data[];
};

struct bitfield_s *bitfield_new(s32 count);
void bitfield_free(struct bitfield_s *self);
void bitfield_setBit(struct bitfield_s *self, s32 index, bool value);
bool bitfield_getBit(struct bitfield_s *self, s32 index);
void bitfield_setAll(struct bitfield_s *self, bool value);

#endif
