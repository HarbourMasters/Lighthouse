#include <cstring>
#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/Romhack/Shared/HackShared.h"

#include "port/Interpolation/FrameInterpolation.h"

extern "C" {
#include "enums.h"
#include "structs.h"
#include "functions.h"
#include "macros.h"
#include "core1/core1.h"
#include "core1/ml.h"
#include "core1/sns.h"
#include "core1/viewport.h"
#include "core2/core2.h"
#include "core2/gc/zoombox.h"
#include "core2/modelRender.h"

typedef struct {
    f32 delay;
    f32 unk4;
    u8* str;
    s16 y;
    u8 portrait;
    u8 unkF;
} PauseMenuOption;

extern struct1Bs D_8036C560[];
extern ActorInfo D_80394C70; // the unused {MARKER_29A_FF_PRIZE, 0x3C6, 0x34C} prize slot
extern ActorInfo gChCubMoggy;
extern u8 D_8036366C[]; // SNS egg collect-burst colours, 6 RGB triples

extern s32 D_80385F30[0x2C]; // item / score counters, indexed by enum item_e
extern f32 D_8037C5B0[3];    // player position
extern s32 D_803726F0[2];    // sparkle sprite config {offset from ASSET_710, size}
extern struct TrackManagerState {
    s16 unk0; // current map's music track
    s16 unk2; // current map's second track
    s16 unk4;
} D_80383340;
extern struct {
    u32 unk0;
    u32 unk4;
    u8 unk8[8];
} D_80383320; // levelSpecificFlags
extern struct {
    u8 D_803832C0[0xD];
    u8 D_803832CD[0xD];
} jiggyscore;
extern struct {
    u8 unk0;
    u8 level;
} D_80383300;
void _levelSpecificFlags_updateCRC1(void);
void _levelSpecificFlags_updateCRC2(void);
}

void TooieJiggyDance_ForceEnable();
void TooieJiggyDance_SetMapSuppression(const s32* maps, s32 count);
void CluckerCutscene_ForceSkip();

namespace {

// Tooie Jiggy Anim is enforced except for these maps
constexpr s32 kDanceSuppressedMaps[] = {
    MAP_16_GV_RUBEES_CHAMBER, MAP_1B_MMM_MAD_MONSTER_MANSION, MAP_1C_MMM_CHURCH,           MAP_1D_MMM_CELLAR,
    MAP_26_MMM_NAPPERS_ROOM,  MAP_34_RBB_ENGINE_ROOM,         MAP_8F_TTC_SHARKFOOD_ISLAND, MAP_92_GV_SNS_CHAMBER,
};

// Pause menu
constexpr s32 kExitEntry = 1; // vanilla "EXIT TO WITCH'S LAIR"
u8 kExitLevelLabel[] = "EXIT LEVEL";

// Lair-lobby table repoints
constexpr s32 kLobbyRemapIndex[] = { 4, 9 };
constexpr s16 kLobbyRemapExit[] = { 2, 3 };

bool SnowGlow_ExitLevelAvailable(s32 level) {
    return (u32)(level - 1) < (u32)(LEVEL_C_BOSS - 1) && level != LEVEL_6_LAIR;
}

void SnowGlow_EnablePauseMenu() {
    for (s32 i = 0; i < (s32)ARRAY_COUNT(kLobbyRemapIndex); i++) {
        D_8036C560[kLobbyRemapIndex[i]].map = MAP_77_GL_RBB_LOBBY;
        D_8036C560[kLobbyRemapIndex[i]].exit = kLobbyRemapExit[i];
    }

    // Listeners run lowest-priority first, force RTL for this hack
    REGISTER_VB_SHOULD(VB_INIT_RETURN_TO_LAIR, EVENT_PRIORITY_HIGH, {
        auto* menu = va_arg(args, PauseMenuOption*);
        menu[kExitEntry].str = kExitLevelLabel;

        if (SnowGlow_ExitLevelAvailable(level_get())) {
            menu[0].y = 46;
            menu[1].y = 76;
            menu[2].y = 106;
            menu[3].y = 136;
            menu[1].delay = 0.1f;
            menu[2].delay = 0.2f;
            menu[3].delay = 0.3f;
            menu[1].portrait = ZOOMBOX_SPRITE_5_GRUNTILDA_2;
            *should = false;
        } else {
            menu[0].y = 55;
            menu[1].y = -100;
            menu[2].y = 90;
            menu[3].y = 125;
            menu[1].delay = 0.3f;
            menu[2].delay = 0.1f;
            menu[3].delay = 0.2f;
            menu[1].portrait = ZOOMBOX_SPRITE_4_BANJO_1;
            *should = true;
        }
    });
}

// Cutscenes
void SnowGlow_EnableCutsceneSkips() {
    REGISTER_VB_SHOULD(VB_CUTSCENE_SKIP_REQUIRE_PROGRESS, EVENT_PRIORITY_NORMAL, { *should = false; });

    REGISTER_VB_SHOULD(VB_GAME_OVER_RETURN_MAP, EVENT_PRIORITY_NORMAL, {
        s32* map = va_arg(args, s32*);
        *map = MAP_77_GL_RBB_LOBBY;
    });
}

// Load resets
void SnowGlow_DisableLevelLoadResets() {
    REGISTER_VB_SHOULD(VB_LEVEL_LOAD_RESET_SCORES, EVENT_PRIORITY_NORMAL, { *should = false; });
    REGISTER_VB_SHOULD(VB_LEVEL_LOAD_RESET_MAP_SETPIECES, EVENT_PRIORITY_NORMAL, { *should = false; });

    REGISTER_VB_SHOULD(VB_MAP_SAVESTATE_CLEAR_ALL, EVENT_PRIORITY_NORMAL,
                       { *should = gsworld_getMap() == MAP_91_FILE_SELECT; });

    REGISTER_VB_SHOULD(VB_RACE_VOID_OUT_FULL_TRANSITION, EVENT_PRIORITY_NORMAL, { *should = true; });
}

// Pause-menu totals rows
void SnowGlow_EnablePauseRowVisibility() {
    REGISTER_VB_SHOULD(VB_PAUSEMENU_ROW_VISIBLE, EVENT_PRIORITY_NORMAL, {
        const s32 selLevel = va_arg(args, s32);
        const s32 row = va_arg(args, s32);
        if (selLevel == LEVEL_6_LAIR) {
            *should = (row == 3);
        }
    });
}

// Dialog gates
constexpr int kSnowGlowSuppressedDialogs[] = {
    ASSET_D96_DIALOG_BEEHIVE_MEET,
    ASSET_D97_DIALOG_JINJO_MEET_YELLOW,
    ASSET_D98_DIALOG_JINJO_MEET_BLUE,
    ASSET_D99_DIALOG_JINJO_MEET_GREEN,
    ASSET_D9A_DIALOG_JINJO_MEET_PINK,
    ASSET_D9B_DIALOG_JINJO_MEET_ORANGE,
    ASSET_D9C_DIALOG_MUSIC_NOTE_MEET,
    ASSET_D9D_DIALOG_MUMBO_TOKEN_MEET,
    ASSET_D9E_DIALOG_BLUE_EGG_MEET,
    ASSET_D9F_DIALOG_RED_FEATHER_MEET,
    ASSET_DA0_DIALOG_GOLD_FEATHER_MEET,
    ASSET_DA1_DIALOG_HONEYCOMB_MEET,
    ASSET_DA3_DIALOG_EXTRA_LIFE_MEET,
    0xF74, // MM 50-note milestone
    0xF75, // past-the-50-note-door taunt
    0xF76, // first-note-in-level reminder
    0xF77, // repeat note-door taunt
};

void SnowGlow_EnableDialogGates() {
    REGISTER_VB_SHOULD(VB_PROGRESS_FLAG_DIALOG, EVENT_PRIORITY_NORMAL, { *should = false; });
    REGISTER_VB_SHOULD(VB_LAIR_GRUNTY_TAUNTS, EVENT_PRIORITY_NORMAL, { *should = false; });
    REGISTER_VB_SHOULD(VB_OVERRIDE_DIALOG_SHOW, EVENT_PRIORITY_LOW, {
        const s32 textId = va_arg(args, s32);
        if ((textId != ASSET_DAA_DIALOG_MUMBO_HAS_ENOUGH_TOKENS &&
             textId != ASSET_DAB_DIALOG_MUMBO_NOT_ENOUGH_TOKENS) ||
            gsworld_getMap() != MAP_48_FP_MUMBOS_SKULL || !jiggyscore_isCollected((enum jiggy_e)3)) {
            return;
        }
        func_80324E38(0.0f, 3);
        *should = true;
    });
}

constexpr int kForceCollectedJiggies[] = { JIGGY_2C_FP_BOGGY_3, JIGGY_2E_FP_PRESENTS };

void SnowGlow_EnableForcedJiggies() {
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        for (const s32 id : kForceCollectedJiggies) {
            jiggyscore_setCollected((enum jiggy_e)id, 1);
        }
    });
}

// Progress-counter caps
struct ProgressCap {
    u8 cap;
    u8 bits;
    s32 flag;
};
constexpr ProgressCap kProgressCaps[] = {
    { 1, 1, 0x5D },  { 2, 2, 0x5E },  { 5, 3, 0x60 }, { 7, 3, 0x63 },  { 8, 4, 0x66 }, { 9, 4, 0x6A },
    { 10, 4, 0x6E }, { 20, 5, 0x72 }, { 0, 1, 0x77 }, { 25, 5, 0x78 }, { 4, 3, 0x7D },
};

void SnowGlow_EnableProgressCaps() {
    REGISTER_LISTENER(OnMapLoadStub, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        for (const auto& entry : kProgressCaps) {
            if (entry.cap < fileProgressFlag_getN((enum file_progress_e)entry.flag, entry.bits)) {
                fileProgressFlag_setN((enum file_progress_e)entry.flag, entry.cap, entry.bits);
            }
        }
    });
}

// Per-level world state
constexpr s32 kLevelSlotStride = 0x88;
constexpr s32 kLevelSlotCount = 14;
constexpr s32 kSlotItems = 0x00;
constexpr s32 kSlotLevelFlags = 0x70;
constexpr s32 kSlotJiggyBits = 0x78;
constexpr s32 kSavedItemBytes = 0x70;
constexpr s32 kJiggyBitBytes = 0xD;
constexpr s32 kAirRefill = 0xE10;
constexpr s32 kRestoredItemOffsets[] = { 0x00, 0x1C, 0x24, 0x10, 0x28, 0x2C, 0x08, 0x20, 0x30, 0x38,
                                         0x48, 0x60, 0x64, 0x8C, 0x68, 0x6C, 0x7C, 0x80, 0x84, 0x88 };

u8 sLevelSlots[kLevelSlotStride * kLevelSlotCount];
bool sLevelSlotsSeeded = false;

u8* SnowGlow_LevelSlot(s32 level) {
    if ((u32)level >= (u32)kLevelSlotCount) {
        level = 0;
    }
    return &sLevelSlots[level * kLevelSlotStride];
}

void SnowGlow_SaveLevelState(s32 level) {
    u8* slot = SnowGlow_LevelSlot(level);
    memcpy(slot + kSlotItems, D_80385F30, kSavedItemBytes);
    memcpy(slot + kSlotLevelFlags, D_80383320.unk8, sizeof(D_80383320.unk8));
    memcpy(slot + kSlotJiggyBits, jiggyscore.D_803832CD, kJiggyBitBytes);
}

void SnowGlow_LoadLevelState(s32 level) {
    u8* slot = SnowGlow_LevelSlot(level);
    for (const s32 offset : kRestoredItemOffsets) {
        memcpy((u8*)D_80385F30 + offset, slot + offset, sizeof(s32));
    }
    D_80385F30[ITEM_17_AIR] = kAirRefill;
    itemPrint_reset();
    memcpy(jiggyscore.D_803832CD, slot + kSlotJiggyBits, kJiggyBitBytes);
    memcpy(D_80383320.unk8, slot + kSlotLevelFlags, sizeof(D_80383320.unk8));
    _levelSpecificFlags_updateCRC1();
    _levelSpecificFlags_updateCRC2();
}

void SnowGlow_EnableLevelStateSlots() {
    REGISTER_VB_SHOULD(VB_LEVEL_LOAD_SAVESTATE_INIT, EVENT_PRIORITY_NORMAL, {
        const s32 prevLevel = va_arg(args, s32);
        *should = false;

        if (gsworld_getMap() == MAP_91_FILE_SELECT) {
            mapSavestate_init();
            itemscore_levelReset((enum level_e)D_80383300.level);
            jiggyscore_clearAllSpawned();
            if (!sLevelSlotsSeeded) {
                sLevelSlotsSeeded = true;
                for (s32 i = 0; i < kLevelSlotCount; i++) {
                    memcpy(&sLevelSlots[i * kLevelSlotStride], D_80385F30, kSavedItemBytes);
                }
            }
            return;
        }

        SnowGlow_SaveLevelState(prevLevel);
        SnowGlow_LoadLevelState(D_80383300.level);
    });
}

// Jiggy totals
constexpr int kJiggiesMovedToFP[] = {
    JIGGY_03_MM_MUMBOS_SKULL, JIGGY_25_BGS_MAZE, JIGGY_02_MM_TICKERS_TOWER, JIGGY_4_MM_JUJU, JIGGY_27_BGS_TIPTUP,
};
constexpr JiggyRelocation kSnowGlowJiggyRelocations[] = {
    { LEVEL_5_FREEZEEZY_PEAK, kJiggiesMovedToFP, ARRAY_COUNT(kJiggiesMovedToFP) },
};

// Honeycomb totals
constexpr s32 kHoneycombsMovedToFP[] = {
    HONEYCOMB_4_TTC_FLOATING_BOX,
    HONEYCOMB_F_RBB_BOAT_HOUSE,
    HONEYCOMB_C_GV_GOBI_3,
    HONEYCOMB_B_GV_CACTUS,
};
constexpr s32 kHoneycombsMovedToMMM[] = {
    HONEYCOMB_1_MM_HILL,       HONEYCOMB_2_MM_JUJU,        HONEYCOMB_3_TTC_UNDERWATER,
    HONEYCOMB_5_CC_UNDERWATER, HONEYCOMB_6_CC_ABOVE_WATER, HONEYCOMB_14_SM_WATERFALL,
};

bool SnowGlow_HoneycombRelocated(s32 index) {
    for (const s32 moved : kHoneycombsMovedToFP) {
        if (moved == index) {
            return true;
        }
    }
    for (const s32 moved : kHoneycombsMovedToMMM) {
        if (moved == index) {
            return true;
        }
    }
    return false;
}

void SnowGlow_EnableHoneycombTotals() {
    REGISTER_VB_SHOULD(VB_HONEYCOMBSCORE_LEVEL_TOTAL, EVENT_PRIORITY_NORMAL, {
        const s32 level = va_arg(args, s32);
        s32* result = va_arg(args, s32*);

        *should = false;
        *result = 0;

        if (level <= 0 || level == LEVEL_6_LAIR || level >= LEVEL_C_BOSS) {
            return;
        }

        // Vanilla's range
        const s32 start = (level < LEVEL_7_GOBIS_VALLEY) ? level * 2 - 1 : level * 2 - 3;
        const s32 end = (level * 2 - 1 == 0x15) ? start + 6 : start + 2;

        s32 total = 0;
        for (s32 i = start; i < end; i++) {
            if (!SnowGlow_HoneycombRelocated(i) && honeycombscore_get((enum honeycomb_e)i)) {
                total++;
            }
        }

        if (level == LEVEL_5_FREEZEEZY_PEAK) {
            for (const s32 moved : kHoneycombsMovedToFP) {
                total += honeycombscore_get((enum honeycomb_e)moved);
            }
        } else if (level == LEVEL_A_MAD_MONSTER_MANSION) {
            for (const s32 moved : kHoneycombsMovedToMMM) {
                total += honeycombscore_get((enum honeycomb_e)moved);
            }
        }

        *result = total;
    });
}

// Per-map SNS pickup dialog
struct SnsEggDialog {
    s32 map;
    s32 textId;
};
constexpr SnsEggDialog kSnsEggDialogs[] = {
    { MAP_3E_RBB_CONTAINER_2, 0xC11 },
    { MAP_25_MMM_WELL, 0xC12 },
    { MAP_92_GV_SNS_CHAMBER, ASSET_DB3_DIALOG_SNS_EGG_1_TEXT },
};

struct SnsEggItem {
    s32 map;
    enum StopNSwop_Item item;
    bool needsUnlocked;
};
constexpr SnsEggItem kSnsEggItems[] = {
    { MAP_3E_RBB_CONTAINER_2, SNS_ITEM_EGG_CYAN, true },       // was MAP_1D_MMM_CELLAR
    { MAP_92_GV_SNS_CHAMBER, SNS_ITEM_EGG_PINK, false },       // swapped with 0x8F
    { MAP_8F_TTC_SHARKFOOD_ISLAND, SNS_ITEM_EGG_BLUE, false }, // swapped with 0x92
    { MAP_2C_MMM_BATHROOM, SNS_ITEM_EGG_GREEN, true },
    { MAP_3F_RBB_CAPTAINS_CABIN, SNS_ITEM_EGG_RED, true },
    { MAP_61_CCW_WINTER_NABNUTS_HOUSE, SNS_ITEM_EGG_YELLOW, true },
};

constexpr s32 kSnsEggTextLo = ASSET_DB3_DIALOG_SNS_EGG_1_TEXT - 1;
constexpr s32 kSnsEggTextHi = ASSET_DB5_DIALOG_ICE_KEY_TEXT;
constexpr f32 kSnsEggDialogDelay = 2.5f;
bool sEmittingSnsDialog = false;

struct SnsEggBurstColor {
    s32 appendage;
    u8 rgb[3];
};
constexpr SnsEggBurstColor kSnsEggBurstColors[] = {
    { 2, { 0x9F, 0xCF, 0xFF } },
    { 5, { 0xFF, 0x7F, 0x00 } },
    { 6, { 0x9F, 0x5F, 0x00 } },
};

void SnowGlow_RecolorSnsEggBursts() {
    for (const auto& entry : kSnsEggBurstColors) {
        u8* row = &D_8036366C[3 * (entry.appendage - 1)];
        row[0] = entry.rgb[0];
        row[1] = entry.rgb[1];
        row[2] = entry.rgb[2];
    }
}

bool SnowGlow_AllSnsCollected() {
    return sns_get_item_state(SNS_ITEM_EGG_PINK, SNS_COLLECTED) &&
           sns_get_item_state(SNS_ITEM_EGG_CYAN, SNS_COLLECTED) && sns_get_item_state(SNS_ITEM_ICE_KEY, SNS_COLLECTED);
}

// Selectively enable SNS
void SnowGlow_EnableStopNSwop() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        gSaveData.sns.uEggPink = 1;
        gSaveData.sns.uEggCyan = 1;
        gSaveData.sns.uIceKey = 1;
    });

    REGISTER_LISTENER(OnSnSItemState, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnSnSItemState*>(event);
        switch (ev->snsItem) {
            case SNS_ITEM_EGG_YELLOW:
            case SNS_ITEM_EGG_RED:
            case SNS_ITEM_EGG_GREEN:
            case SNS_ITEM_EGG_BLUE:
                ev->result = false;
                event->Cancelled = true;
                break;
            default:
                break;
        }
    });

    REGISTER_VB_SHOULD(VB_OVERRIDE_SNS_MAP_CHECK, EVENT_PRIORITY_NORMAL, {
        const s32 site = va_arg(args, s32);
        const s32 map = gsworld_getMap();

        if (site == SNS_MAP_CHECK_PICKUP) {
            for (const auto& entry : kSnsEggItems) {
                if (entry.map == map) {
                    sns_set_item_and_update_payload(entry.item, SNS_COLLECTED, 1);
                    break;
                }
            }
            *should = true;
            return;
        }

        Actor* self = va_arg(args, Actor*);
        bool keep = false;
        for (const auto& entry : kSnsEggItems) {
            if (entry.map != map) {
                continue;
            }
            keep = !sns_get_item_state(entry.item, SNS_COLLECTED) &&
                   (!entry.needsUnlocked || sns_get_item_state(entry.item, SNS_UNLOCKED));
            break;
        }
        if (!keep && self != NULL && self->marker != NULL) {
            marker_despawn(self->marker);
        }
        *should = true;
    });

    REGISTER_VB_SHOULD(VB_OVERRIDE_TIMED_DIALOGUE, EVENT_PRIORITY_NORMAL, {
        const s32 textId = va_arg(args, s32);
        if (sEmittingSnsDialog || textId < kSnsEggTextLo || textId > kSnsEggTextHi) {
            return;
        }
        const s32 map = gsworld_getMap();
        for (const auto& entry : kSnsEggDialogs) {
            if (entry.map != map) {
                continue;
            }
            *should = true;
            sEmittingSnsDialog = true;
            func_80324DBC(kSnsEggDialogDelay, (enum asset_e)entry.textId, 0x20, NULL, NULL, NULL, NULL);
            sEmittingSnsDialog = false;
            return;
        }
    });
}

Actor* (*sMoggyDraw)(ActorMarker*, Gfx**, Mtx**, Vtx**) = nullptr;
constexpr s32 kMoggyAppendageEnd = 0xF; // hide 1..0xE, leaving only the base mesh

// Don't spawn Gobi2
void SnowGlow_SuppressGobi2() {
    REGISTER_LISTENER(OnActorSpawn, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnActorSpawn*>(event);
        if (ev->actorId == ACTOR_131_GOBI_2) {
            ev->result = nullptr;
            event->Cancelled = true;
        }
    });
}

// Sparkle projectiles use asset 0x70F instead of the yellow sparkle
void SnowGlow_RepointSparkleSprite() {
    D_803726F0[0] = -1;
}

// Music carries across warps between these three maps
constexpr int kVillageMusicMaps[] = { MAP_3F_RBB_CAPTAINS_CABIN, MAP_64_CCW_WINTER_NABNUTS_WATER_SUPPLY,
                                      MAP_38_RBB_CONTAINER_3 };
constexpr WarpMusicGroup kSnowGlowMusicGroups[] = {
    { kVillageMusicMaps, ARRAY_COUNT(kVillageMusicMaps) },
};

// ------------------------------------------------- Ambience + proximity music

constexpr s32 kAmbienceAllChanMaps[] = {
    0x05, 0x0C, 0x14, 0x15, 0x25, 0x27, 0x28, 0x29, 0x2A, 0x2D, 0x2E,
    0x36, 0x38, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x48, 0x64, 0x77,
};

struct ProximityMusicZone {
    s32 map;
    f32 anchor[3];
    f32 outer;
    f32 inner;
    s32 track;
};
constexpr ProximityMusicZone kProximityMusicZones[] = {
    { MAP_3A_RBB_BOSS_BOOM_BOX, { 500.0f, 100.0f, 300.0f }, 400.0f, 150.0f, 0x7B },
    { MAP_3B_RBB_STORAGE_ROOM, { -500.0f, 100.0f, 280.0f }, 400.0f, 150.0f, 0x7C },
    { MAP_3C_RBB_KITCHEN, { 500.0f, 100.0f, -350.0f }, 400.0f, 150.0f, 0x7D },
};

void SnowGlow_AmbienceUpdate(s32* playerPos, s32* trackId) {
    const s32 map = gsworld_getMap();

    // Channel masks
    if (map == MAP_34_RBB_ENGINE_ROOM) {
        core1_ce60_setChanMask(player_getWaterState() == BSWATERGROUP_2_UNDERWATER ? 0x800 : 0x43FE);
    } else if (map == MAP_91_FILE_SELECT) {
        core1_ce60_setChanMaskWithTransitionSpeed(gameSelect_getGameNumber() == 0 ? 0x200 : 0x1FF, 0.5f);
    } else {
        for (const s32 m : kAmbienceAllChanMaps) {
            if (m == map) {
                core1_ce60_setChanMask(player_getWaterState() == BSWATERGROUP_2_UNDERWATER ? 0x8000 : 0x7FFF);
                break;
            }
        }
    }

    // Proximity music
    const ProximityMusicZone* activeZone = nullptr;
    f32 zoneDistance = 0.0f;
    bool onZoneMap = false;
    for (const auto& zone : kProximityMusicZones) {
        if (zone.map != map) {
            continue;
        }
        f32 anchor[3] = { zone.anchor[0], zone.anchor[1], zone.anchor[2] };
        zoneDistance = ml_vec3f_distance(anchor, D_8037C5B0);
        if (zoneDistance < zone.outer) {
            activeZone = &zone;
            break;
        }
        onZoneMap = true;
    }

    if (activeZone != nullptr) {
        if (D_80383340.unk2 != activeZone->track) {
            musicTrack_release((comusic_e)D_80383340.unk2);
            musicTrack_load((comusic_e)activeZone->track);
        }
        D_80383340.unk2 = (s16)activeZone->track;
        if (zoneDistance < activeZone->inner) {
            trackId[2] = 0;
            trackId[3] = gcMusic_getDefaultVolumeForTrack((comusic_e)activeZone->track);
            core1_ce60_func_8024A9EC(0);
        }
        trackId[0] = D_80383340.unk0;
        trackId[1] = activeZone->track;
    } else if (onZoneMap) {
        trackId[2] = gcMusic_getDefaultVolumeForTrack((comusic_e)D_80383340.unk0);
        trackId[3] = 0;
        core1_ce60_func_8024A9EC(0);
        player_getPosition_s32(playerPos);
    }
}

void SnowGlow_EnableAmbience() {
    REGISTER_VB_SHOULD(VB_AMBIENCE_MAP_UPDATE, EVENT_PRIORITY_NORMAL, {
        s32* playerPos = va_arg(args, s32*);
        s32* trackId = va_arg(args, s32*); // [main track, second track, main vol, second vol]
        *should = false;
        SnowGlow_AmbienceUpdate(playerPos, trackId);
    });
}

// ------------------------------------------------------ Stop 'N' Swop pause page

struct SnsPageSlot {
    s32 item;      // SNS item / egg-model appendage
    s32 angleSlot; // which unk3E/unk4C entry tracks this slot
    s16 spinRate;  // degrees per second, written every frame like the blob does
    f32 x;         // viewport x offset
    f32 yOffset;   // model-position y offset
    f32 rotZ;
    f32 scale;
    bool iceKey;
};
constexpr SnsPageSlot kSnsPageSlots[] = {
    { SNS_ITEM_EGG_PINK, 5, 79, -290.0f, 100.0f, 0.0f, 0.8f, false },
    { SNS_ITEM_EGG_CYAN, 6, -31, 290.0f, 100.0f, 0.0f, 0.8f, false },
    { SNS_ITEM_ICE_KEY, 0, 79, 0.0f, 0.0f, 45.0f, 1.0f, true },
};
constexpr s32 kSnsGhostAlpha = 0x50;

void SnowGlow_DrawSnsPage(Gfx** gfx, Mtx** mtx, s32 snsAlpha, s16* angles, BKModelBin* eggModel, BKModelBin* keyModel) {
    const f32 dt = time_getDelta();
    for (const auto& slot : kSnsPageSlots) {
        FrameInterpolation_RecordOpenChild("sgv_sns", (uintptr_t)slot.item);
        const f32 angle = mlNormalizeAngle((f32)angles[slot.angleSlot] + (f32)slot.spinRate * dt);
        angles[slot.angleSlot] = (s16)angle;

        viewport_backupState();
        f32 vpPos[3] = { slot.x, 0.0f, 1000.0f };
        f32 vpRot[3] = { 0.0f, 0.0f, 0.0f };
        viewport_setPosition_vec3f(vpPos);
        viewport_setRotation_vec3f(vpRot);
        viewport_update();
        viewport_setRenderViewportAndPerspectiveMatrix(gfx, mtx);

        f32 pos[3] = { 0.0f, 0.0f, 0.0f };
        f32 rot[3] = { 0.0f, angle, slot.rotZ };
        f32 offset[3] = { 0.0f, slot.yOffset, 0.0f };
        for (s32 j = 1; j < 7; j++) {
            modelRender_setAppendageVisibility(j, j == slot.item);
        }
        modelRender_setDepthMode(MODEL_RENDER_DEPTH_NONE);
        const bool collected = sns_get_item_state((enum StopNSwop_Item)slot.item, SNS_COLLECTED);
        modelRender_setAlpha(collected ? snsAlpha : MIN(snsAlpha, kSnsGhostAlpha));
        modelRender_draw(gfx, mtx, pos, rot, slot.scale, offset, slot.iceKey ? keyModel : eggModel);
        viewport_restoreState();
        viewport_setRenderViewportAndPerspectiveMatrix(gfx, mtx);
        FrameInterpolation_RecordCloseChild();
    }
}

void SnowGlow_EnableSnsPausePage() {
    REGISTER_VB_SHOULD(VB_PAUSEMENU_SNS_ITEMS, EVENT_PRIORITY_NORMAL, {
        s32* items = va_arg(args, s32*);
        *items = 1;
        (void)should;
    });

    REGISTER_VB_SHOULD(VB_PAUSEMENU_SNS_DRAW, EVENT_PRIORITY_NORMAL, {
        Gfx** gfx = va_arg(args, Gfx**);
        Mtx** mtx = va_arg(args, Mtx**);
        const s32 snsAlpha = va_arg(args, s32);
        s16* angles = va_arg(args, s16*);
        BKModelBin* eggModel = va_arg(args, BKModelBin*);
        BKModelBin* keyModel = va_arg(args, BKModelBin*);
        *should = false;
        SnowGlow_DrawSnsPage(gfx, mtx, snsAlpha, angles, eggModel, keyModel);
    });
}

} // namespace

// Recycled unused 4th FF prize slot
extern "C" void SnowGlow_PrizeUpdate(Actor* thisx) {
    if (SnowGlow_AllSnsCollected()) {
        marker_despawn(thisx->marker);
    }
}

// Moggy's slot is reused with a replaced ASSET_44C
extern "C" Actor* SnowGlow_MoggyDraw(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx) {
    for (s32 i = 1; i < kMoggyAppendageEnd; i++) {
        modelRender_setAppendageVisibility(i, FALSE);
    }
    return sMoggyDraw(marker, gfx, mtx, vtx);
}

void RegisterSnowGlowVillagePatches() {
    D_80394C70.update_func = SnowGlow_PrizeUpdate;
    sMoggyDraw = gChCubMoggy.draw_func;
    gChCubMoggy.draw_func = SnowGlow_MoggyDraw;

    TooieJiggyDance_ForceEnable();
    TooieJiggyDance_SetMapSuppression(kDanceSuppressedMaps, (s32)ARRAY_COUNT(kDanceSuppressedMaps));

    SnowGlow_EnablePauseMenu();
    SnowGlow_DisableLevelLoadResets();
    SnowGlow_EnableCutsceneSkips();
    SnowGlow_EnablePauseRowVisibility();
    SnowGlow_EnableDialogGates();
    HackShared_EnableDialogSuppression(kSnowGlowSuppressedDialogs);
    SnowGlow_EnableForcedJiggies();
    SnowGlow_EnableProgressCaps();
    SnowGlow_EnableLevelStateSlots();
    HackShared_EnableJiggyRelocation(kSnowGlowJiggyRelocations, kForceCollectedJiggies);
    SnowGlow_EnableHoneycombTotals();
    SnowGlow_RecolorSnsEggBursts();
    SnowGlow_EnableStopNSwop();
    SnowGlow_EnableSnsPausePage();
    SnowGlow_SuppressGobi2();
    CluckerCutscene_ForceSkip();
    SnowGlow_RepointSparkleSprite();
    SnowGlow_EnableAmbience();
    HackShared_EnableWarpMusicGroups(kSnowGlowMusicGroups);
    HackShared_EnableMumboReward();
    HackShared_EnableForceAbilitiesUsed(kAllUsedAbilities);
    port_overrideRomhackHoneycombsPerWorld(6);
}
