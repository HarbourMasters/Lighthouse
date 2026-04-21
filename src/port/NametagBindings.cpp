// BK-specific bindings for the portable Nametag overlay: projection, FB
// dimensions, tick-event listeners, and the asset-id -> label table.

#include <libultraship/libultraship.h>

#include "nametag.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ui/cvar_prefixes.h"
#include "port/ObjectExtension/ObjectExtension.h"

extern "C" {
#include "enums.h"
#include "core1/viewport.h"
    extern int gFramebufferWidth;
    extern int gFramebufferHeight;
}

#define CVAR CVAR_DEVELOPER_TOOLS("Nametags")

struct NoteIdentity {
    uint32_t index;
};

uint32_t noteIndex = 1;

namespace {

    struct NametagTableEntry {
        int32_t assetId;
        const char* label;
        float yOffset;
    };

    // Asset-id -> nametag lookup. Shared across the actor / prop / sprite-prop paths.
    constexpr NametagTableEntry sNametagTable[] = {
        { ASSET_350_MODEL_TERMITE, "Termite", 150.0f },         { ASSET_3C5_MODEL_GRUBLIN, "Grublin", 150.0f },
        { ASSET_353_MODEL_BIGBUTT, "BigButt", 150.0f },         { ASSET_3C0_MODEL_JINJO_BLUE, "Blue Jinjo", 150.0f },
        { ASSET_3C2_MODEL_JINJO_GREEN, "Green Jinjo", 150.0f }, { ASSET_3BC_MODEL_JINJO_ORANGE, "Orange Jinjo", 150.0f },
        { ASSET_3C1_MODEL_JINJO_PINK, "Pink Jinjo", 150.0f },   { ASSET_3BB_MODEL_JINJO_YELLOW, "Yellow Jinjo", 150.0f },
        { ASSET_6D6_SPRITE_MUSIC_NOTE, "Note", 80.0f },         { ASSET_41A_SPRITE_MUMBO_TOKEN, "Mumbo Token", 80.0f },
    };

    const NametagTableEntry* LookupNametag(int32_t assetId) {
        for (const auto& entry : sNametagTable) {
            if (entry.assetId == assetId) {
                return &entry;
            }
        }
        return nullptr;
    }
    
    static bool inRange(float x, float y, float z) {
        float pos[3] = {x, y, z};
        return viewport_getDistance(pos) < CVarGetFloat(CVAR_DEVELOPER_TOOLS("NametagDist"), 3000.0f);
    }

    static bool inRange(int16_t pos[3]) {
        return inRange((float) pos[0], (float) pos[1], (float) pos[2]);
    }
    static std::vector<Prop*> registeredNotes;
    void NTBody(IEvent* event) {
        Nametag::Clear();
        for (Prop* prop : registeredNotes) {
            if (inRange(prop->modelProp.unk4)) {
                char buf[30];
                NoteIdentity* ident = ObjectExtension::GetInstance().Get<NoteIdentity>(prop);
                sprintf(buf, "Note %d", ident->index);
                const char* label = buf;
                Nametag::Push((float) prop->modelProp.unk4[0], (float) prop->modelProp.unk4[1] + 80.0f, (float) prop->modelProp.unk4[2], label);
            }
        }
    }

    void InitNametagBindings() {
        Nametag::SetProjectFn(&viewport_func_8024E030);
        Nametag::SetNativeFramebufferSize(&gFramebufferWidth, &gFramebufferHeight);
        Nametag::RegisterOverlay();

        //REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_HIGH, [](IEvent* event) { Nametag::Clear(); });

        // Forwards through OnNametagDraw so non-dispatched actors can register too.
        //REGISTER_LISTENER(OnActorTick, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        //    auto* ev = reinterpret_cast<OnActorTick*>(event);
        //    if (ev->actor == nullptr || ev->actor->actor_info == nullptr) {
        //        return;
        //    }
        //    const NametagTableEntry* entry = LookupNametag(ev->actor->actor_info->modelId);
        //    if (entry == nullptr) {
        //        char buf[30];
        //        sprintf(buf, "%d", ev->actor->actor_info->actorId);
        //        const char* buf2 = buf;
        //        CALL_EVENT(OnNametagDraw, ev->actor, buf2, 100.0f);
        //        return;
        //    }
        //    CALL_EVENT(OnNametagDraw, ev->actor, entry->label, entry->yOffset);
        //    });

        //REGISTER_LISTENER(OnPropTick, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        //    auto* ev = reinterpret_cast<OnPropTick*>(event);
        //    if (ev->marker == nullptr || ev->position == nullptr) {
        //        //char buf[30];
        //        //sprintf(buf, "%d", ev->marker->modelId);
        //        //const char* buf2 = buf;
        //        //Nametag::Push(ev->position[0], ev->position[1] + 100.0f, ev->position[2], buf2);
        //        return;
        //    }
        //    const NametagTableEntry* entry = LookupNametag(ev->marker->modelId);
        //    if (entry == nullptr) {
        //        return;
        //    }
        //    Nametag::Push(ev->position[0], ev->position[1] + entry->yOffset, ev->position[2], entry->label);
        //    });

        //REGISTER_LISTENER(OnSpritePropTick, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        //    auto* ev = reinterpret_cast<OnSpritePropTick*>(event);
        //    if (ev->position == nullptr) {
        //        return;
        //    }
        //    const NametagTableEntry* entry = LookupNametag(ev->assetId);
        //    if (entry == nullptr) {
        //        char buf[30];
        //        sprintf(buf, "%d", ev->assetId);
        //        const char* buf2 = buf;
        //        Nametag::Push(ev->position[0], ev->position[1] + 100.0f, ev->position[2], buf2);
        //        return;
        //    }
        //    char buf[30];
        //    sprintf(buf, "%s", entry->label);
        //    if (ev->assetId == ASSET_6D6_SPRITE_MUSIC_NOTE && )
        //    Nametag::Push(ev->position[0], ev->position[1] + entry->yOffset, ev->position[2], entry->label);
        //    });

        //REGISTER_LISTENER(OnNametagDraw, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        //    auto* ev = reinterpret_cast<OnNametagDraw*>(event);
        //    if (ev->actor == nullptr || ev->label == nullptr || !inRange(ev->actor->position[0], ev->actor->position[1], ev->actor->position[2])) {
        //        return;
        //    }
        //    Nametag::Push(ev->actor->position[0], ev->actor->position[1] + ev->yOffset, ev->actor->position[2], ev->label);
        //    });

        COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_HIGH, CVarGetInteger(CVAR, 0), NTBody);
    }
    void OEBody(IEvent* event) {
        auto* ev = reinterpret_cast<OnPropInit*>(event);
        if (ev->propPtr->markerFlag) {

        } else if (ev->propPtr->unk8_1) {

        } else {
            if ((s32)(ev->propPtr->spriteProp.spriteId + 0x572) == ASSET_6D6_SPRITE_MUSIC_NOTE) {
                ObjectExtension::GetInstance().Set<NoteIdentity>(ev->propPtr, {noteIndex});
                registeredNotes.push_back(ev->propPtr);
                noteIndex++;
            }
        }
    }
    static ObjectExtension::Register<NoteIdentity> RegisterNoteIdentity;
    void InitObjectExtensions() {
        REGISTER_LISTENER(OnPropInit, EVENT_PRIORITY_HIGH, OEBody);

        REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_HIGH, [](IEvent* event) {
            for (auto prop : registeredNotes) {
                if (ObjectExtension::GetInstance().Has<NoteIdentity>(prop)) {
                    ObjectExtension::GetInstance().Remove<NoteIdentity>(prop);
                }
            }
            registeredNotes.clear();
            noteIndex = 1;
        });
    }

} // namespace

static RegisterShipInitFunc initNametagBindings(InitNametagBindings, { CVAR });

static RegisterShipInitFunc initObjectExtensions(InitObjectExtensions);