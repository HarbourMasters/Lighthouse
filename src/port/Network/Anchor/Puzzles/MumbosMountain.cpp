#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "MM/ch/jujuhitbox.h"
#include "MM/ch/orangepad.h"
}

#include "port/Patches/Patches.h"

// A teammate's knock is playing out on the Jujus, until one of ours.
static bool sJujuReplay = false;

// The team's knocked totems, one bit each.
static s32 JujusKnocked() {
    const s32 bits = port_puzzleStep_get(ANCHOR_PUZZLE_MM_JUJU) & 0xF;
    return (bits & 1) + ((bits >> 1) & 1) + ((bits >> 2) & 1) + ((bits >> 3) & 1);
}

static bool IsJujuTeammateKnock(Actor* actor) {
    return sJujuReplay && actor != NULL && actor->modelCacheIndex == ACTOR_59_JUJU;
}

void AnchorPuzzles_RegisterMumbosMountain(bool connected) {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_11_JUJU_CTRL, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* hitbox = ev->actor;
        auto* local = (ActorLocal_JujuHitbox*)&hitbox->local;
        if (!hitbox->volatile_initialized) {
            // The hitbox spawns only the totems still standing.
            if (Anchor::GetInstance()->IsWorldSyncActive()) {
                local->unk4 = JujusKnocked();
                if (local->unk4 >= 4) {
                    marker_despawn(hitbox->marker);
                    *ev->should = false;
                }
            }
            return;
        }
        // A teammate knocked the next one down.
        if (hitbox->state == 3 && local->unk4 < JujusKnocked()) {
            hitbox->state = 1;
            local->unk4++;
            sJujuReplay = true;
            func_803892A8(local->jujus);
        }
    });

    // A teammate's knock doesn't spawn a second jiggy, and shakes our camera and plays
    // its sounds only within earshot. Always on.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsJujuTeammateKnock(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_SPAWN_PUZZLE_JIGGY, EVENT_PRIORITY_NORMAL, true, {
        if (IsJujuTeammateKnock(va_arg(args, Actor*))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsJujuTeammateKnock(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    // Always on, with the hooks that read it. A teammate's knock replays without a step, so any step here is ours.
    COND_HOOK(OnPuzzleStep, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (((OnPuzzleStep*)event)->puzzleId == ANCHOR_PUZZLE_MM_JUJU) {
            sJujuReplay = false;
        }
    });
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sJujuReplay = false; });

    // A pad a teammate already hit stays hit.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_57_ORANGE_PAD, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* pad = ((ShouldActorUpdate*)event)->actor;
        if (pad->state != ORANGE_PAD_STATE_HIT &&
            port_breakable_isBroken((s32)gsworld_getMap(), (s32)pad->marker->id, (s32)pad->position[0],
                                    (s32)pad->position[1], (s32)pad->position[2])) {
            pad->state = ORANGE_PAD_STATE_HIT;
        }
    });
}
