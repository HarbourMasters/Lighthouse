#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "MMM/ch/cemetarypot.h"
#include "MMM/ch/tumblar.h"

void func_80387654(ActorMarker* marker);
void func_803895D8(ActorMarker* caller, enum asset_e text_id, s32 arg2);
void func_8038A140(s32 arg0);
void organMinigame_setState(s32 next_state);
}

#include "port/Patches/Patches.h"

// A teammate collected Napper's jiggy here, and his scene plays out.
static bool sNapperTeammateCollect = false;

void AnchorPuzzles_MarkNapperTeammateCollect() {
    sNapperTeammateCollect = true;
}

static bool IsTeammateStep(Actor* actor) {
    if (actor == NULL) {
        return false;
    }
    switch (actor->modelCacheIndex) {
        case ACTOR_39_NAPPER:
            return sNapperTeammateCollect;
        case ACTOR_114_CHURCH_DOOR:
            return port_mapFlag_wasSetRemotely(MMM_SPECIFIC_FLAG_0_UNKNOWN);
        default:
            return false;
    }
}

void AnchorPuzzles_RegisterMadMonsterMansion(bool connected) {
    // A pot a teammate flowered flowers here too, without counting toward ours.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_25_CEMETARY_POT, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* pot = ((ShouldActorUpdate*)event)->actor;
        if (pot->state == FLOWER_POT_STATE_1_IDLE &&
            port_breakable_isBroken((s32)gsworld_getMap(), (s32)pot->marker->id, (s32)pot->position[0],
                                    (s32)pot->position[1], (s32)pot->position[2])) {
            subaddie_set_state(pot, FLOWER_POT_STATE_2_FLOWERED);
            anctrl_setPlaybackType(pot->anctrl, ANIMCTRL_ONCE);
        }
    });

    // A teammate collected Tumblar's jiggy, whose freed marker may belong to another actor now. He vanishes too.
    COND_VB_SHOULD(VB_TUMBLAR_HOLDS_JIGGY, EVENT_PRIORITY_NORMAL, connected, {
        if (jiggyscore_isCollected(JIGGY_62_MMM_TUMBLAR)) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_TUMBLAR_BREAK, EVENT_PRIORITY_NORMAL, connected, {
        if (va_arg(args, s32) == TUMBLAR_STATE_0_IDLE && jiggyscore_isCollected(JIGGY_62_MMM_TUMBLAR)) {
            func_8038A140(6); // our run ends, or the hourglass times us out
            *should = true;
        }
    });

    // A teammate collected Napper's jiggy, whose freed marker may belong to another actor now.
    COND_VB_SHOULD(VB_NAPPER_SET_JIGGY_POSITION, EVENT_PRIORITY_NORMAL, connected, {
        if (Anchor::GetInstance()->IsWorldSyncActive() && jiggyscore_isCollected(JIGGY_5D_MMM_NAPPER)) {
            *should = false;
        }
    });

    // A teammate's step replayed here goes without our camera, and shows its dialogs and plays its sounds only within
    // earshot. Always on: a replay that starts while connected plays out if the connection drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        if (IsTeammateStep(va_arg(args, Actor*))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_DIALOG, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor) && !Anchor_InEarshot(actor->position)) {
            // Napper leaves from the dialog's callback.
            if (actor->modelCacheIndex == ACTOR_39_NAPPER) {
                func_803895D8(NULL, (enum asset_e)0, 0);
            }
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    // Always on, with the hooks that read it.
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sNapperTeammateCollect = false; });

    // A teammate beat Motzhand and his jiggy is out: our game ends, outside its dialogs.
    COND_VB_SHOULD(VB_ORGAN_GAME_IDLE, EVENT_PRIORITY_NORMAL, connected, {
        const s32 state = va_arg(args, s32);
        ActorMarker* motzhand = va_arg(args, ActorMarker*);
        if ((state == 1 || state == 3) && motzhand != NULL && jiggyscore_isSpawned(JIGGY_60_MMM_MOTZHAND)) {
            organMinigame_setState(7);
            func_80387654(motzhand);
            *should = true;
        }
    });
}
