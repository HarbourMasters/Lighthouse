#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

extern "C" {
s32 item_adjustByDiffWithHud(enum item_e item, s32 diff);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JINJOS].optionValue

void Rando::ObjectBehavior::InitJinjoBehavior() {
    REGISTER_LISTENER(OnSetJiggyList, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;

        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        for (auto& pool : Rando::Logic::shuffledPool) {
            if (!pool.obtained) {
                continue;
            }

            if (Rando::StaticData::Checks[pool.shuffledCheckId].worldId != ev->levelId) {
                continue;
            }

            if (pool.randoItemId >= RI_JINJO_BLUE && pool.randoItemId <= RI_JINJO_YELLOW) {
                int32_t jinjoMarkerId =
                    GetJinjoActorMarkerId((actor_e)Rando::StaticData::Items[pool.randoItemId].actorId);
                item_adjustByDiffWithHud(ITEM_12_JINJOS, (1 << ((jinjoMarkerId + 6) & 0x1F)));
            }
        }
    })

    COND_VB_SHOULD(VB_UPDATE_JINJO_HUD, EVENT_PRIORITY_NORMAL, OPTION_ENABLED, {
        f32* position = va_arg(args, f32*);

        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        *should = true;
    })

    COND_VB_SHOULD(VB_SET_JINJO_COUNT, EVENT_PRIORITY_NORMAL, OPTION_ENABLED, {
        f32* position = va_arg(args, f32*);

        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        *should = true;
    })
}
