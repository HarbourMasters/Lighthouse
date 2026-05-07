#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/enhancements/events/PortEnhancements.h"
#include "port/enhancements/events/hooks/Events.h"

extern "C" {
enum map_e gsworld_getMap(void);
enum level_e map_getLevel(enum map_e map);

void item_inc(enum item_e item);
}

// TODO: SWAP TO RANDO_SAVE_OPTIONS
#define CVAR_NAME Rando::StaticData::Options[RO_SHUFFLE_MUSIC_NOTES].cvar
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void Rando::ObjectBehavior::InitMusicNoteBehavior() {
    REGISTER_LISTENER(OnSetJiggyList, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;

        // if (!IS_RANDO) {
        //     return;
        // }

        if (CVAR) {
            for (auto& pool : Rando::Logic::shuffledPool) {
                if (Rando::StaticData::Checks[pool.shuffleCheckId].worldId != ev->levelId) {
                    continue;
                }
            
                if (pool.randoItemId == RI_MUSIC_NOTE) {
                    if (pool.obtained) {
                        item_inc(ITEM_C_NOTE);
                    }
                }
            }
        }
    })
}
