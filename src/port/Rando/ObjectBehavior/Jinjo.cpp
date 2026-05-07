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

s32 item_adjustByDiffWithHud(enum item_e item, s32 diff);
}

// TODO: SWAP TO RANDO_SAVE_OPTIONS
#define CVAR_NAME Rando::StaticData::Options[RO_SHUFFLE_JINJOS].cvar
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void Rando::ObjectBehavior::InitJinjoBehavior() {
    REGISTER_LISTENER(OnSetJiggyList, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;

        // if (!IS_RANDO) {
        //     return;
        // }

        if (CVAR) {
            for (auto& pool : Rando::Logic::shuffledPool) {
                if (!pool.obtained) {
                    continue;
                }
            
                if (Rando::StaticData::Checks[pool.shuffleCheckId].worldId != ev->levelId) {
                    continue;
                }
            
                if (pool.randoItemId >= RI_JINJO_BLUE && pool.randoItemId <= RI_JINJO_YELLOW) {
                    int32_t jinjoMarkerId = GetJinjoActorMarkerId((actor_e)Rando::StaticData::Items[pool.randoItemId].actorId);
                    item_adjustByDiffWithHud(ITEM_12_JINJOS, (1 << ((jinjoMarkerId + 6) & 0x1F)));
                }
            }
        }
    })

    COND_VB_SHOULD(VB_SET_JINJO_COUNT, EVENT_PRIORITY_NORMAL, CVAR, {
        f32* position = va_arg(args, f32*);
        // if (!IS_RANDO) {
        //     return;
        // }

        *should = false;

        // int16_t actorPos[3];
        // actorPos[0] = (int16_t)position[0];
        // actorPos[1] = (int16_t)position[1];
        // actorPos[2] = (int16_t)position[2];
        // 
        // RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(actorPos[0], actorPos[1], actorPos[2]);
        // if (randoCheckId == RC_UNKNOWN) {
        //     return;
        // }
        // 
        // Rando::StaticData::RandoShuffledPool shuffledObject = Rando::Logic::GetShuffledObject(randoCheckId);
        // if (shuffledObject.randoCheckId == RC_UNKNOWN) {
        //     return;
        // }
        // 
        // if (Rando::StaticData::Checks[shuffledObject.shuffleCheckId].worldId != map_getLevel(gsworld_getMap())) {
        //     return;
        // }
        // 
        // *should = false;
    })
}
