#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/Shared/HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "macros.h"
}

void CluckerCutscene_ForceSkip();

namespace {

constexpr int kHub = MAP_6C_GL_RED_CAULDRON_ROOM;

constexpr TransitionPair kCheatosTransitions[] = {
    { kHub, MAP_2_MM_MUMBOS_MOUNTAIN, 0x4, 0x9 },    { kHub, MAP_7_TTC_TREASURE_TROVE_COVE, 0x4, 0x9 },
    { kHub, MAP_D_BGS_BUBBLEGLOOP_SWAMP, 0x4, 0x9 }, { kHub, MAP_1B_MMM_MAD_MONSTER_MANSION, 0x4, 0x9 },
    { kHub, MAP_27_FP_FREEZEEZY_PEAK, 0x4, 0x9 },    { kHub, MAP_31_RBB_RUSTY_BUCKET_BAY, 0x4, 0x9 },
    { MAP_2_MM_MUMBOS_MOUNTAIN, kHub, 0xD, 0xE },    { MAP_7_TTC_TREASURE_TROVE_COVE, kHub, 0xD, 0xE },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, kHub, 0xD, 0xE }, { MAP_1B_MMM_MAD_MONSTER_MANSION, kHub, 0xD, 0xE },
    { MAP_27_FP_FREEZEEZY_PEAK, kHub, 0xD, 0xE },    { MAP_31_RBB_RUSTY_BUCKET_BAY, kHub, 0xD, 0xE },
};

constexpr int kJiggyToBakery[] = { 0x4A };
constexpr JiggyRelocation kCheatosJiggies[] = {
    { LEVEL_A_MAD_MONSTER_MANSION, kJiggyToBakery, ARRAY_COUNT(kJiggyToBakery) },
};

constexpr int kLibraryMaps[] = { MAP_6A_GL_TTC_AND_CC_PUZZLE, kHub };
constexpr int kPeakMaps[] = { MAP_27_FP_FREEZEEZY_PEAK, MAP_7F_FP_WOZZAS_CAVE };
constexpr WarpMusicGroup kCheatosMusicGroups[] = {
    { kLibraryMaps, ARRAY_COUNT(kLibraryMaps) },
    { kPeakMaps, ARRAY_COUNT(kPeakMaps) },
};

constexpr int kCheatosSuppressedDialogs[] = {
    0xF75,
    0xF76,
    0xF77,
};

} // namespace

void RegisterCheatosChallengesPatches() {
    HackShared_EnableTransitionPairs(kCheatosTransitions);
    HackShared_EnableJiggyRelocation(kCheatosJiggies);
    HackShared_SetJiggyLevelCap(2);
    HackShared_EnableWarpMusicGroups(kCheatosMusicGroups);
    HackShared_EnablePodiumCheck();
    HackShared_EnablePuzzleDepositClamp();
    HackShared_EnableFileSelectGameOver();
    HackShared_EnableDialogSuppression(kCheatosSuppressedDialogs);
    HackShared_EnableForceAbilitiesUsed(kAllUsedAbilities);
    CluckerCutscene_ForceSkip();

    // Disable the jigsaw transition and banjo drone state on level entry
    REGISTER_VB_SHOULD(VB_LEVEL_ENTERED_FROM_LAIR, EVENT_PRIORITY_NORMAL, { *should = false; });

    // Two jiggies a level
    REGISTER_VB_SHOULD(VB_LAST_JIGGY_COUNT, EVENT_PRIORITY_NORMAL, {
        s32* count = va_arg(args, s32*);
        *count = 1;
        (void)should;
    });

    // Cheato's Library only shows its time
    REGISTER_VB_SHOULD(VB_PAUSEMENU_ROW_VISIBLE, EVENT_PRIORITY_NORMAL, {
        const s32 selLevel = va_arg(args, s32);
        const s32 row = va_arg(args, s32);
        if (selLevel == LEVEL_6_LAIR) {
            *should = (row == 3);
        }
    });

    REGISTER_VB_SHOULD(VB_HEALTH_HUD_SHOW, EVENT_PRIORITY_NORMAL, {
        const s32 map = va_arg(args, s32);
        if (map == kHub) {
            *should = false;
        }
    });
}
