#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

extern "C" {
void item_inc(enum item_e item);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUSIC_NOTES].optionValue

void Rando::ObjectBehavior::InitMusicNoteBehavior() {
    REGISTER_LISTENER(OnSetJiggyList, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;

        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        int32_t currentNotes = 0;
        for (auto& pool : Rando::Logic::shuffledPool) {
            if (Rando::StaticData::Checks[pool.shuffledCheckId].worldId != ev->levelId) {
                continue;
            }

            if (pool.randoItemId == RI_MUSIC_NOTE) {
                if (pool.obtained) {
                    currentNotes++;
                }
            }
        }

        item_set(ITEM_C_NOTE, currentNotes);
    })
}
