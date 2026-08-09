#pragma once

extern "C" {
#include "core2/abilityprogress.h"
#include "prop.h"
}

// ------------------------------------------------------------------ Small gates

constexpr ability_used kAllUsedAbilities[] = {
    ABILITY_USED_JUMP,       ABILITY_USED_FLAP,  ABILITY_USED_FLIP,  ABILITY_USED_SWIM, ABILITY_USED_CLIMB,
    ABILITY_USED_BEAK_BARGE, ABILITY_USED_SLIDE, ABILITY_USED_EGG,   ABILITY_USED_FLY,  ABILITY_USED_SHOCK,
    ABILITY_USED_PECK,       ABILITY_USED_CLAW,  ABILITY_USED_TWIRL,
};

void HackShared_EnableNoteSignSuppression(int signActorId);
void HackShared_EnableDialogSuppression(const int* dialogIds, int count);
void HackShared_EnableForceAbilitiesUsed(const ability_used* moves, int count);

template <int N> inline void HackShared_EnableDialogSuppression(const int (&dialogIds)[N]) {
    HackShared_EnableDialogSuppression(dialogIds, N);
}

template <int N> inline void HackShared_EnableForceAbilitiesUsed(const ability_used (&moves)[N]) {
    HackShared_EnableForceAbilitiesUsed(moves, N);
}

// ------------------------------------------------------- Relocated jiggies

// Hacks that move jiggies between worlds keep vanilla's ids but retally them: each
// relocated id is dropped from the level whose id-range it falls in, and counted
// under the level that now hosts it.
struct JiggyRelocation {
    int toLevel;
    const int* ids;
    int count;
};

// `alsoExcluded` drops ids from every level's tally without re-adding them anywhere
// (for jiggies a hack force-collects, which would otherwise inflate a total).
void HackShared_EnableJiggyRelocation(const JiggyRelocation* groups, int groupCount, const int* alsoExcluded = nullptr,
                                      int excludedCount = 0);

template <int N> inline void HackShared_EnableJiggyRelocation(const JiggyRelocation (&groups)[N]) {
    HackShared_EnableJiggyRelocation(groups, N);
}

template <int N, int M>
inline void HackShared_EnableJiggyRelocation(const JiggyRelocation (&groups)[N], const int (&alsoExcluded)[M]) {
    HackShared_EnableJiggyRelocation(groups, N, alsoExcluded, M);
}

// ------------------------------------------------------------------ Warp music

// Music keeps playing across a warp when the current and destination maps are
// in the same group.
struct WarpMusicGroup {
    const int* maps;
    int count;
};

void HackShared_EnableWarpMusicGroups(const WarpMusicGroup* groups, int groupCount);

template <int N> inline void HackShared_EnableWarpMusicGroups(const WarpMusicGroup (&groups)[N]) {
    HackShared_EnableWarpMusicGroups(groups, N);
}

// ------------------------------------------------------------------ Mumbo reward

// Instead of transformations, Mumbo rewards a jiggy
void HackShared_EnableMumboReward();

// ------------------------------------------------------------------ Proximity dialogs

struct ProximityDialogPage {
    float anchor[3];
    float radius;
    int textId;
    int dialogFlags;
    unsigned char word;     // which sDialogShown word tracks this page
    unsigned short doneBit; // bit within that word; 0 means "guarded by token instead"
    int token;              // mumbotoken to guard on / bank (-1 = none)
    const float* dialogPos;
    int skipIfJiggy;   // skip if this jiggy is collected (>0)
    int needFlag;      // skip unless this FILEPROG flag is set (>0)
    int needFlagClear; // skip if this FILEPROG flag is set (>0)
};

struct ProximityDialogMap {
    int map;
    const ProximityDialogPage* pages;
    int pageCount;
    // Optional per-map hook run before the page loop. Return false to skip this
    // map's pages this frame; may also perform side effects. nullptr = always run.
    bool (*gate)();
};

// Run one frame of checks against a caller-owned table (used by the storybook).
void ProximityDialogs_Run(const ProximityDialogMap* maps, int count);

// Register a GameFrameUpdate that runs the given world table every frame.
void ProximityDialogs_Enable(const ProximityDialogMap* maps, int count);

// Deduce the count from a fixed table so callers pass just the table.
template <int N> inline void ProximityDialogs_Enable(const ProximityDialogMap (&maps)[N]) {
    ProximityDialogs_Enable(maps, N);
}

// Shown-bit access for gate() callbacks that re-arm a page.
bool ProximityDialogs_IsShown(int word, unsigned bits);
void ProximityDialogs_ClearShown(int word, unsigned bits);

// ------------------------------------------------------------------ Storybook

struct StorybookConfig {
    const ProximityDialogMap* pages; // book pages, one entry per book map
    int pageCount;                   // number of book maps
    int resumeDest;                  // MAP<<8 | ENTRY warp used when resuming a save
    int newGameSeenToken;            // mumbotoken the first book banks (-1 = none)
};

void Storybook_Enable(const StorybookConfig& cfg);

// ------------------------------------------------------------------ Stealth noise

struct StealthNoiseConfig {
    int map;
    float zoneMin;
    float zoneMax;
    float innerMin;
    float innerMax;
    int caughtTextId;
    int escapeWarpIndex;
    void (*escapeWarpFallback)(NodeProp*, ActorMarker*);
    int firstCatchToken;
};

void StealthNoise_Enable(const StealthNoiseConfig& cfg);
void StealthNoise_AddBurst(float amount, float seconds);
