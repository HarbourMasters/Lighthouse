#pragma once

extern "C" {
#include "core2/abilityprogress.h"
}

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
