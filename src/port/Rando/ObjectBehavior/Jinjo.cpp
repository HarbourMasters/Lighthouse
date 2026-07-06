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
    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;

        for (auto& location : Rando::Logic::shuffledPool) {
            if (!location.obtained) {
                continue;
            }

            Rando::StaticData::RandoStaticItem randoItem = Rando::StaticData::Items[location.randoItemId];

            if (randoItem.worldId != ev->levelId) {
                continue;
            }

            if (randoItem.randoItemType == RITYPE_JINJO) {
                int32_t jinjoMarkerId = GetJinjoActorMarkerId((actor_e)randoItem.actorId);
                item_adjustByDiffWithHud(ITEM_12_JINJOS, (1 << ((jinjoMarkerId + 6) & 0x1F)));
            }
        }
    })

    COND_VB_SHOULD(VB_UPDATE_JINJO_HUD, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, { *should = true; })

    COND_VB_SHOULD(VB_SET_JINJO_COUNT, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, { *should = true; })
}
