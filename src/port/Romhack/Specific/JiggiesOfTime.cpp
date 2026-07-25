/*
 * Jiggies of Time Fun Facts
 *
 * The Collywobble actor is marker-swapped to prevent a spawn. This is rarely seen anyway, since BB enforces
 * file select as the default boot. With vanilla, if you were to view the intro sequence after a game over,
 * you'd see a stray Collywobble spawned during the concert when it shouldn't be there at all.
 *
 * An unused vanilla marker 0x54 spawns ASSET_50B_MODEL_X_BARREL_TOP, which in Jiggies of Time is model-swapped
 * with a random honeycomb drop.
 *
 */

#include <libultraship/bridge.h>
#include "port/Romhack/Shared/HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"

extern ActorInfo chXBarrelTop;
extern ActorInfo gChVegetablesCollywobbleB;
}

void RegisterJiggiesOfTimePatches() {
    chXBarrelTop.markerId = 0x54;
    gChVegetablesCollywobbleB.markerId = MARKER_1F1_GRUNTLING_BLACK;

    // JoT's note signs are repurposed Red Question Marks.
    // Source: https://github.com/Mr-Wiseguy/JiggiesOfTimeRecomp/blob/main/src/note_signs.c
    HackShared_EnableNoteSignSuppression(ACTOR_54_RED_QUESTION_MARK);
}
