#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "spdlog/spdlog.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUSIC_NOTES].optionValue

void Rando::ObjectBehavior::InitPropBehavior() {
    COND_VB_SHOULD(VB_OVERRIDE_PROP_SPAWN, EVENT_PRIORITY_NORMAL, true, {
        s16* spawnPosition = va_arg(args, s16*);

        if (!IS_RANDO) {
            return;
        }

        if (!OPTION_ENABLED) {
            return;
        }

        int32_t position[3];
        position[0] = spawnPosition[0];
        position[1] = spawnPosition[1];
        position[2] = spawnPosition[2];

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(position[0], position[1], position[2]);
        if (randoCheckId == RC_UNKNOWN) {
            *should = false;
            return;
        }

        if (!Rando::Logic::IsCheckShuffled(randoCheckId)) {
            *should = false;
            return;
        }

        Actor* randoCustomActor = CustomObject::ShouldCreateCustomActorEX(randoCheckId, position, true);
        *should = true;
    })
}
