#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "MM/ch/jujuhitbox.h"
#include "MM/ch/orangepad.h"
}

#include "port/Patches/Patches.h"

// The team's knocked totems, one bit each.
static s32 JujusKnocked() {
    const s32 bits = port_puzzleStep_get(ANCHOR_PUZZLE_MM_JUJU) & 0xF;
    return (bits & 1) + ((bits >> 1) & 1) + ((bits >> 2) & 1) + ((bits >> 3) & 1);
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
            func_803892A8(local->jujus);
        }
    });

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
