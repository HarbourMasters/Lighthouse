#include <libultraship/bridge.h>
#include <algorithm>
#include <cstring>
#include <vector>
#include "port/Romhack/Shared/HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "macros.h"
#include "actor.h"
#include "core2/model.h"
#include "core2/print.h"

extern ActorInfo chCellarHatch;
extern ActorInfo D_803914B8;
extern ActorInfo chCrackedSkylight;
extern f32 s_player_velocity[3];
extern OSContPad sInputs[MAXCONTROLLERS];

void func_802D3CE8(Actor* thisx);
}

namespace {

// Holding A in turbo trainers keeps the player rising
void LotCJ_TurboTrainerLift() {
    if (!modelAppendages_hideTurboTrainers() || !(sInputs[0].button & A_BUTTON)) {
        return;
    }
    // Only the high half of the float is written, so the low bits carry over
    u32 bits;
    std::memcpy(&bits, &s_player_velocity[1], sizeof(bits));
    bits = (bits & 0xFFFF) | 0x43DC0000;
    std::memcpy(&s_player_velocity[1], &bits, sizeof(bits));
}

// Two marked groups of church vertices rise while the player holds two or more jiggies
BKModelBin* sChurchModel = nullptr;
std::vector<std::pair<Vtx*, f32>> sChurchVerts;

void LotCJ_RaiseChurch() {
    BKModelBin* bin = mapModel_getModelBin(0);
    if (bin == nullptr) {
        return;
    }
    // The original heights are kept so the model can drop back if it stays cached
    if (bin != sChurchModel) {
        sChurchModel = bin;
        sChurchVerts.clear();
        BKVertexList* vertexList = modelbin_getVtxList(bin);
        const s32 count = vtxList_getVtxCount(vertexList);
        for (s32 i = 0; i < count; i++) {
            Vtx* vertex = &vertexList->vertices[i];
            if (vertex->v.cn[1] == 0xC9 || vertex->v.cn[1] == 0x96) {
                sChurchVerts.push_back({ vertex, (f32)vertex->v.ob[1] });
            }
        }
    }
    const bool raised = item_getCount(ITEM_26_JIGGY_TOTAL) >= 2;
    for (const auto& [vertex, originalY] : sChurchVerts) {
        if (raised) {
            vertex->v.ob[1] = (vertex->v.cn[1] == 0xC9) ? 0x300 : 0x350;
        } else {
            vertex->v.ob[1] = originalY;
        }
    }
}

struct VelocityBox {
    s32 map;
    s16 min[3];
    s16 max[3];
    s16 push[3];
};

constexpr VelocityBox kVelocityBoxes[] = {
    { MAP_7F_FP_WOZZAS_CAVE, { -2700, -100, 300 }, { -30, 1000, 3700 }, { 66, 0, -101 } },
    { MAP_45_CCW_AUTUMN, { -39, 802, 1300 }, { 81, 1102, 1420 }, { 0, 17274, 0 } },
};
constexpr f32 kMaxBoxVelocity = 600.0f;

// Currents and updrafts: the first box the player is inside pushes them every frame
void LotCJ_PushFromBoxes() {
    f32 pos[3];
    player_getPosition(pos);
    for (const VelocityBox& box : kVelocityBoxes) {
        if (box.map != gsworld_getMap()) {
            continue;
        }
        bool inside = true;
        for (int i = 0; i < 3; i++) {
            inside = inside && pos[i] >= box.min[i] && pos[i] <= box.max[i];
        }
        if (!inside) {
            continue;
        }
        for (int i = 0; i < 3; i++) {
            s_player_velocity[i] = std::clamp(s_player_velocity[i] + box.push[i], -kMaxBoxVelocity, kMaxBoxVelocity);
        }
        return;
    }
}

struct CreditLine {
    s32 x;
    s32 y;
    const char* text;
};

// clang-format off
constexpr CreditLine kCredits0[] = {
    {  84,  16, "TOOLS USED :" },
    {  56,  52, "BANJO'S BACKPACK" },
    {  83,  76, "HXD" },
    { 172,  76, "BK2BT" },
    {  64, 100, "BLENDER" },
    { 176, 104, "GIMP" },
    {  56, 128, "N64 SOUND TOOL" },
    {  28, 152, "N64 SOUNDBANK TOOL" },
    {  68, 176, "N64 MIDI TOOL" },
    {  60, 200, "WUMBA'S WIGWAM" },
};
constexpr CreditLine kCredits1[] = {
    {  60,  28, "MAIN HACK IDEA :" },
    {  92,  52, "JACKSONG13" },
    {  84,  88, "LEVEL DESIGN :" },
    {  92, 112, "JACKSONG13" },
    {  92, 148, "MODELERS :" },
    {  92, 172, "JACKSONG13" },
};
constexpr CreditLine kCredits2[] = {
    {  20,  36, "USEFUL TIPS AND TRICKS :" },
    { 116,  72, "BYNINE" },
    {  84,  96, "MARK KURKO" },
    {  81, 120, "THATCOWGUY" },
    {  62, 144, "SPACEOMEGA5000" },
};
constexpr CreditLine kCredits3[] = {
    {  12,  36, "CERTAIN CUSTOM MODELS :" },
    { 116,  64, "BYNINE" },
    {  84,  88, "MARK KURKO" },
    {  92, 112, "JACKSONG13" },
    {  36, 152, "CERTAIN MODELS USED :" },
    {  76, 176, "VGRESOURCE.COM" },
};
constexpr CreditLine kCredits4[] = {
    {  36,  36, "CUSTOM CODED STUFF :" },
    { 116,  72, "BYNINE" },
    {  81,  96, "THATCOWGUY" },
    {  62, 120, "SPACEOMEGA5000" },
    {  92, 144, "JACKSONG13" },
    { 104, 168, "TRENAVIX" },
};
constexpr CreditLine kCredits5[] = {
    {  44,  36, "CERTAIN MUSIC USED :" },
    {  92,  60, "VGMUSIC.COM" },
    {  28,  92, "MUSIC ARRANGEMENTS :" },
    {  92, 116, "JACKSONG13" },
    {  72, 148, "SOUND EFFECTS :" },
    {  92, 172, "JACKSONG13" },
};
constexpr CreditLine kCredits6[] = {
    {  24,  32, "TEXTURES AND SPRITES :" },
    { 112,  56, "RWP.COM" },
    {  92,  80, "JACKSONG13" },
    { 108, 116, "TESTERS :" },
    {  36, 140, "JACKSON'S WONDERFUL" },
    { 116, 164, "SIBLINGS" },
};
constexpr CreditLine kCredits7[] = {
    {  68,  36, "SPECIAL THANKS :" },
    {  62,  68, "SPACEOMEGA5000" },
    {  81,  92, "THATCOWGUY" },
    { 132, 116, "SKILL" },
    { 110, 140, "SUBDRAG" },
};
constexpr CreditLine kCredits8[] = {
    {  52,  36, "HACK BY JACKSONG13" },
    {  44,  80, "ALL RIGHTS BELONG TO" },
    {  72, 104, "RARE, MICROSOFT" },
    {  84, 128, "AND NINTENDO" },
    {  40, 172, "THANKS FOR PLAYING!" },
};
// clang-format on

struct CreditPage {
    const CreditLine* lines;
    int count;
};

constexpr CreditPage kCreditPages[] = {
    { kCredits0, ARRAY_COUNT(kCredits0) }, { kCredits1, ARRAY_COUNT(kCredits1) }, { kCredits2, ARRAY_COUNT(kCredits2) },
    { kCredits3, ARRAY_COUNT(kCredits3) }, { kCredits4, ARRAY_COUNT(kCredits4) }, { kCredits5, ARRAY_COUNT(kCredits5) },
    { kCredits6, ARRAY_COUNT(kCredits6) }, { kCredits7, ARRAY_COUNT(kCredits7) }, { kCredits8, ARRAY_COUNT(kCredits8) },
};
constexpr u16 kCreditPageFrames = 300;
constexpr f32 kCreditTextScale = -1.0001953f;

u16 sCreditFrames = 0;

// The credits roll on map 0x13, one page every 300 frames, and the last page stays up
void LotCJ_RollCredits() {
    if (gsworld_getMap() != MAP_13_GV_MEMORY_GAME) {
        sCreditFrames = 0;
        return;
    }
    int page = sCreditFrames / kCreditPageFrames;
    if (page >= ARRAY_COUNT(kCreditPages)) {
        page = ARRAY_COUNT(kCreditPages) - 1;
    }
    for (int i = 0; i < kCreditPages[page].count; i++) {
        const CreditLine& line = kCreditPages[page].lines[i];
        print_bold_overlapping(line.x, line.y, kCreditTextScale, (u8*)line.text);
    }
    sCreditFrames++;
}

// The hack runs these from this actor's update, so only on maps that place it, and twice where there are two
void LotCJ_HatchUpdate(Actor* thisx) {
    func_802D3CE8(thisx);
    LotCJ_TurboTrainerLift();
    if (gsworld_getMap() == MAP_1C_MMM_CHURCH) {
        LotCJ_RaiseChurch();
    }
    LotCJ_PushFromBoxes();
    LotCJ_RollCredits();
}

} // namespace

void RegisterLegendOfTheCrystalJiggyPatches() {
    chCellarHatch.update_func = LotCJ_HatchUpdate;
    // This actor becomes a touch pickup
    D_803914B8.markerId = 0x54;
    // Skylights are placed as collectible notes
    chCrackedSkylight.markerId = MARKER_5F_MUSIC_NOTE;
}
