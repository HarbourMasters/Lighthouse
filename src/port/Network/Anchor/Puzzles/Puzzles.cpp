#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"

void AnchorPuzzles_Register(bool connected) {
    AnchorPuzzles_RegisterMumbosMountain(connected);
    AnchorPuzzles_RegisterTreasureTroveCove(connected);
    AnchorPuzzles_RegisterClankersCavern(connected);
    AnchorPuzzles_RegisterBubblegloopSwamp(connected);
    AnchorPuzzles_RegisterFreezeezyPeak(connected);
    AnchorPuzzles_RegisterGobisValley(connected);
    AnchorPuzzles_RegisterMadMonsterMansion(connected);
    AnchorPuzzles_RegisterRustyBucketBay(connected);
    AnchorPuzzles_RegisterClickClockWood(connected);

    // A teammate's throw lands as a copy here, the only one of these actors to ask. Always on: one thrown while
    // connected lands if the connection drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (actor != NULL &&
            (actor->modelCacheIndex == ACTOR_2A_GOLD_BULLION || actor->modelCacheIndex == ACTOR_2A2_CATERPILLAR ||
             actor->modelCacheIndex == ACTOR_2A9_ACORN) &&
            !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
}
