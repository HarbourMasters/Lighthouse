#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "actor.h"

extern ActorArray* suBaddieActorArray;

// Bold world-name font internals
extern s32 D_80380AE8;  // active font slot (1 == bold)
extern s32 D_80380AF0;  // monospaced flag
extern char D_80380AB0; // previous letter
f32 print_calculateLetterXPos(u8 letter, f32* xPtr, f32* yPtr, f32 arg3);
}

#define CVAR_NOTE_RETENTION CVAR_ENHANCEMENT("Gameplay.NoteRetention")

namespace {

void ApplyNoteSignHooks();
f32 MeasureBoldNameWidth(const char* s);
void ApplyPauseNameCentering();
void ApplyDialogSuppression();

int sNoteSignActorId = -1;
const int* sSuppressedDialogs = nullptr;
int sSuppressedDialogCount = 0;

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

// Center bold font in pause menu
f32 MeasureBoldNameWidth(const char* s) {
    const s32 savedSlot = D_80380AE8;
    const s32 savedMono = D_80380AF0;
    const char savedPrev = D_80380AB0;
    D_80380AE8 = 1;
    D_80380AF0 = 0;
    D_80380AB0 = 0;
    f32 x = 0.0f;
    f32 y = 0.0f;
    for (const char* p = s; *p != '\0'; ++p) {
        print_calculateLetterXPos((u8)(unsigned char)*p, &x, &y, 1.05f);
    }
    D_80380AE8 = savedSlot;
    D_80380AF0 = savedMono;
    D_80380AB0 = savedPrev;
    return x;
}

void ApplyPauseNameCentering() {
    COND_VB_SHOULD(VB_PAUSEMENU_LEVEL_NAME_X, EVENT_PRIORITY_NORMAL, port_getRomhackIdentifier() != nullptr, {
        s32* x = va_arg(args, s32*);
        va_arg(args, s32);
        const char* vanillaName = va_arg(args, const char*);
        const char* romhackName = va_arg(args, const char*);
        if (romhackName != nullptr && vanillaName != nullptr) {
            const f32 shift = (MeasureBoldNameWidth(vanillaName) - MeasureBoldNameWidth(romhackName)) * 0.5f;
            *x += (s32)(shift >= 0.0f ? shift + 0.5f : shift - 0.5f);
        }
        (void)should;
    });
}

// Suppress a caller-supplied set of dialogs
void ApplyDialogSuppression() {
    COND_VB_SHOULD(VB_OVERRIDE_DIALOG_SHOW, EVENT_PRIORITY_NORMAL, sSuppressedDialogCount > 0, {
        const s32 textId = va_arg(args, s32);
        for (int i = 0; i < sSuppressedDialogCount; i++) {
            if (sSuppressedDialogs[i] == textId) {
                *should = true;
                break;
            }
        }
    });
}

RegisterShipInitFunc noteSignInitFunc(ApplyNoteSignHooks, { CVAR_NOTE_RETENTION });
RegisterShipInitFunc pauseNameCenterInit(ApplyPauseNameCentering, { "BOOT" });

} // namespace

void HackShared_EnableNoteSignSuppression(int signActorId) {
    sNoteSignActorId = signActorId;
    ApplyNoteSignHooks();
}

void HackShared_EnableDialogSuppression(const int* dialogIds, int count) {
    sSuppressedDialogs = dialogIds;
    sSuppressedDialogCount = count;
    ApplyDialogSuppression();
}
