#include <libultraship/bridge.h>
#include <cstring>
#include <vector>

#include "port/Romhack/RomhackConfig.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Patches/Patches.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "actor.h"
#include "core2/model.h"
#include "core2/modelRender.h"

extern s16 D_8039347C[12];
}

namespace {

constexpr s32 kNoteDoorCount = 12;
// Where each door's number list starts, as its geo selector loads it
constexpr s32 kNoteDoorNumberList[kNoteDoorCount] = { 45, 66, 93, 120, 147, 174, 201, 228, 255, 282, 309, 336 };
constexpr s32 kTwoDigitList = 45;    // door 0 (50)
constexpr s32 kThreeDigitList = 255; // door 8 (828)
constexpr s32 kTwoDigitLength = 21;
constexpr s32 kThreeDigitLength = 27;
constexpr s32 kDigitImageCmd[3] = { 3, 14, 20 }; // each digit's G_SETTIMG within a template
constexpr s32 kDigitVertexCmd = 12;              // the first quad's G_VTX in the three-digit template
// The texture for each digit 0-8; the model has no '9'
constexpr s32 kDigitTexture[9] = { 2, 4, 5, 6, 7, 8, 9, 10, 11 };
constexpr s32 kSixTexture = 9;
constexpr size_t kDigitBytes = 32 * 32 * 4; // RGBA32 32x32

bool sComposeNoteDoors = false;
bool sNoteDoorsComposed = false;
bool sNoteDoorsUnmatched = false;
std::vector<u64> sComposedNoteDoorList;
Gfx sNoteDoorNumberLists[kNoteDoorCount][kThreeDigitLength];
u8 sNineTexture[kDigitBytes];
uintptr_t sDigitImages[10];

u8 GfxOpcode(const Gfx& gfx) {
    return (u8)(gfx.words.w0 >> 24);
}

// Hacks can replace the note door model, so only compose one laid out like vanilla's
bool NoteDoorModelMatches(BKGfxList* gfxList, BKVertexList* vertexList, BKTextureList* textureList) {
    if (gfxList->size <= (u32)kNoteDoorNumberList[kNoteDoorCount - 1] + 1 || vertexList->count < 168 ||
        textureList->count < 12) {
        return false;
    }
    for (s32 start : kNoteDoorNumberList) {
        if (GfxOpcode(gfxList->list[start]) != 0xB6) { // G_CLEARGEOMETRYMODE
            return false;
        }
    }
    for (s32 cmd : kDigitImageCmd) {
        if (GfxOpcode(gfxList->list[kThreeDigitList + cmd]) != 0xFD) { // G_SETTIMG
            return false;
        }
    }
    return GfxOpcode(gfxList->list[kTwoDigitList + kDigitImageCmd[0]]) == 0xFD &&
           GfxOpcode(gfxList->list[kTwoDigitList + kDigitImageCmd[1]]) == 0xFD &&
           GfxOpcode(gfxList->list[kThreeDigitList + kDigitVertexCmd]) == 0x04; // G_VTX
}

// The '9' is the '6' with its bytes reversed, which turns the glyph upside down.
bool FindDigitImages(BKGfxList* gfxList, BKTextureList* textureList) {
    uintptr_t byTexture[12] = {};
    for (u32 i = 0; i < gfxList->size; i++) {
        const uintptr_t image = gfxList->list[i].words.w1;
        if (GfxOpcode(gfxList->list[i]) != 0xFD || image == 0 || (image & 1) != 0) {
            continue;
        }
        const char* path = (const char*)image;
        if (strncmp(path, "__OTR__", 7) != 0) {
            continue;
        }
        const char* tag = nullptr;
        for (const char* found = strstr(path, "_tex_"); found != nullptr; found = strstr(found + 1, "_tex_")) {
            tag = found;
        }
        const int texture = tag != nullptr ? atoi(tag + 5) : -1;
        if (texture >= 0 && texture < 12) {
            byTexture[texture] = image;
        }
    }
    for (s32 digit = 0; digit < 9; digit++) {
        sDigitImages[digit] = byTexture[kDigitTexture[digit]];
        if (sDigitImages[digit] == 0) {
            return false;
        }
    }
    const u8* six = textureList_getDataPtr(textureList) + textureList_getTextureInfo(textureList, kSixTexture)->offset;
    for (size_t i = 0; i < kDigitBytes; i++) {
        sNineTexture[i] = six[kDigitBytes - 1 - i];
    }
    sDigitImages[9] = (uintptr_t)sNineTexture;
    return true;
}

void BuildNoteDoorNumberList(Gfx* out, const Gfx* model, u32 cost) {
    u32 digits = 0;
    u32 rest = cost;
    do {
        digits++;
        rest /= 10;
    } while (rest != 0);

    const u32 ones = cost % 10;
    const u32 tens = cost / 10 % 10;
    const u32 hundreds = cost / 100 % 10;
    if (digits == 1) {
        // The three-digit template up to its first quad
        memcpy(out, &model[kThreeDigitList], (kDigitVertexCmd + 2) * sizeof(Gfx));
        out[kDigitVertexCmd].words.w1 -= 4 * sizeof(Vtx); // the lone-digit quad, vertices 116-119
        gSPEndDisplayList(&out[kDigitVertexCmd + 2]);
        out[kDigitImageCmd[0]].words.w1 = sDigitImages[ones];
    } else if (digits == 2) {
        memcpy(out, &model[kTwoDigitList], kTwoDigitLength * sizeof(Gfx));
        out[kDigitImageCmd[0]].words.w1 = sDigitImages[tens];
        out[kDigitImageCmd[1]].words.w1 = sDigitImages[ones];
    } else {
        // Four digits keep the last three
        memcpy(out, &model[kThreeDigitList], kThreeDigitLength * sizeof(Gfx));
        out[kDigitImageCmd[0]].words.w1 = sDigitImages[hundreds];
        out[kDigitImageCmd[1]].words.w1 = sDigitImages[tens];
        out[kDigitImageCmd[2]].words.w1 = sDigitImages[ones];
    }
}

// The shared vertex list gets the composed digit positions once per load of the model.
// Fixed layouts, not centered: one digit uses 116-119 (a copy of 124-127, moved), three digits get new X and S on
// 124-131, two digits keep door 0's quads.
void LayOutNoteDoorDigits(BKVertexList* vertexList) {
    Vtx* v = vertexList->vertices;
    if (v[116].v.ob[0] == 182.0f && v[124].v.ob[0] == 158.0f) {
        return;
    }
    memcpy(&v[116], &v[124], 4 * sizeof(Vtx));
    v[116].v.ob[0] = 182.0f;
    v[117].v.ob[0] = 182.0f;
    v[117].v.ob[1] = 168.0f;
    v[118].v.ob[0] = 46.0f;
    v[118].v.ob[1] = 168.0f;
    v[119].v.ob[0] = 46.0f;
    v[124].v.ob[0] = v[125].v.ob[0] = 158.0f;
    v[124].v.tc[0] = v[125].v.tc[0] = 0x800;
    v[126].v.ob[0] = v[127].v.ob[0] = 64.0f;
    v[126].v.tc[0] = v[127].v.tc[0] = 0;
    v[128].v.ob[0] = v[129].v.ob[0] = 248.0f;
    v[128].v.tc[0] = v[129].v.tc[0] = 0x800;
    v[130].v.ob[0] = v[131].v.ob[0] = 152.0f;
    v[130].v.tc[0] = v[131].v.tc[0] = 0;
}

BKGfxList* ComposeNoteDoors(BKModelBin* bin) {
    if (bin == nullptr || sNoteDoorsUnmatched) {
        return nullptr;
    }
    BKGfxList* gfxList = modelbin_getGfxList(bin);
    BKVertexList* vertexList = modelbin_getVtxList(bin);
    if (!sNoteDoorsComposed) {
        BKTextureList* textureList = modelbin_getTextureList(bin);
        if (!NoteDoorModelMatches(gfxList, vertexList, textureList) || !FindDigitImages(gfxList, textureList)) {
            sNoteDoorsUnmatched = true;
            return nullptr;
        }
        // +1 for the G_ENDDL the model factory appends; u64 storage keeps the copy's Gfx 8-byte aligned
        const size_t bytes = sizeof(BKGfxList) + (gfxList->size + 1) * sizeof(Gfx);
        sComposedNoteDoorList.assign((bytes + sizeof(u64) - 1) / sizeof(u64), 0);
        BKGfxList* composed = (BKGfxList*)sComposedNoteDoorList.data();
        memcpy(composed, gfxList, bytes);
        // Each door's list becomes a call to its composed number and an end; the geo selectors don't change
        for (s32 door = 0; door < kNoteDoorCount; door++) {
            BuildNoteDoorNumberList(sNoteDoorNumberLists[door], gfxList->list, (u16)port_getNoteDoorCost(door));
            gSPDisplayList(&composed->list[kNoteDoorNumberList[door]], sNoteDoorNumberLists[door]);
            gSPEndDisplayList(&composed->list[kNoteDoorNumberList[door] + 1]);
        }
        sNoteDoorsComposed = true;
    }
    LayOutNoteDoorDigits(vertexList);
    return (BKGfxList*)sComposedNoteDoorList.data();
}

void ApplyNoteDoorNumbers() {
    // Hide the baked number mesh on doors the hack retunes
    COND_VB_SHOULD(VB_NOTEDOOR_DRAW_NUMBER, EVENT_PRIORITY_NORMAL, port_isRomhack() && !sComposeNoteDoors, {
        const s32 noteDoorIdx = va_arg(args, s32);
        if (noteDoorIdx >= 1 && port_getRomhackNoteDoor(noteDoorIdx - 1) >= 0) {
            *should = false;
        }
    });

    // Draw the substitute mesh
    COND_VB_SHOULD(VB_NOTEDOOR_DRAW_NUMBER, EVENT_PRIORITY_NORMAL, sComposeNoteDoors, {
        const s32 noteDoorIdx = va_arg(args, s32);
        ActorMarker* marker = va_arg(args, ActorMarker*);
        BKModelBin* bin = (BKModelBin*)assetcache_get((enum asset_e)marker->modelId);
        BKGfxList* composed = ComposeNoteDoors(bin);
        assetcache_release(bin);
        if (composed != nullptr) {
            modelRender_setDisplayList(composed);
        } else if (noteDoorIdx >= 1 && port_getRomhackNoteDoor(noteDoorIdx - 1) >= 0) {
            *should = false;
        }
    });
}

RegisterShipInitFunc noteDoorNumberInit(ApplyNoteDoorNumbers, { "BOOT" });

} // namespace

extern "C" int port_getNoteDoorCost(int doorIdx) {
    const int cost = port_getRomhackNoteDoor(doorIdx);
    return cost >= 0 ? cost : D_8039347C[doorIdx];
}

extern "C" void port_enableNoteDoorComposer(void) {
    sComposeNoteDoors = true;
    ApplyNoteDoorNumbers();
}
