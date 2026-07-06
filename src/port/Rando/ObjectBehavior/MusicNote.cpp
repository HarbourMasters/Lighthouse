#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUSIC_NOTES].optionValue

void Rando::ObjectBehavior::InitMusicNoteBehavior() {
    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;
        int32_t currentNotes = 0;

        for (auto& location : Rando::Logic::shuffledPool) {
            Rando::StaticData::RandoStaticItem randoItem = Rando::StaticData::Items[location.randoItemId];

            if (randoItem.worldId != ev->levelId) {
                continue;
            }

            if (randoItem.randoItemType == RITYPE_MUSIC_NOTE) {
                if (location.obtained) {
                    currentNotes++;
                }
            }
        }

        item_adjustByDiffWithoutHud(ITEM_C_NOTE, currentNotes - item_getCount(ITEM_C_NOTE));
    })
}
