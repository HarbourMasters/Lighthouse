#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"

extern ActorArray* suBaddieActorArray;
}

#define CVAR_NOTE_RETENTION CVAR_ENHANCEMENT("Gameplay.NoteRetention")

namespace {

int sNoteSignActorId = -1;

// Romhacks that have note signs and Bottles explainers don't need them when
// note saving is turned on. Suppress them.
void ApplyNoteSignHooks() {
    const bool active = sNoteSignActorId >= 0 && CVarGetInteger(CVAR_NOTE_RETENTION, 0);

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, active, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnActorSpawn*>(event);
        if (ev->actorId == sNoteSignActorId) {
            ev->result = nullptr;
            event->Cancelled = true;
        }
    });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, active, [](IEvent*) {
        if (suBaddieActorArray == nullptr) {
            return;
        }
        Actor* start = suBaddieActorArray->data;
        Actor* end = start + suBaddieActorArray->cnt;
        for (Actor* actor = start; actor < end; actor++) {
            if (actor->marker == nullptr || actor->marker->id != MARKER_B7_TUTORIAL_BOTTLES ||
                actor->actorTypeSpecificField != 8) {
                continue;
            }
            if (actor->partnerActor != nullptr) {
                marker_despawn(actor->partnerActor);
            }
            marker_despawn(actor->marker);
        }
    });
}

RegisterShipInitFunc noteSignInitFunc(ApplyNoteSignHooks, { CVAR_NOTE_RETENTION });

} // namespace

void HackShared_EnableNoteSignSuppression(int signActorId) {
    sNoteSignActorId = signActorId;
    ApplyNoteSignHooks();
}
