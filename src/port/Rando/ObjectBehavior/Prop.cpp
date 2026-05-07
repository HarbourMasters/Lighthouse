#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/enhancements/events/PortEnhancements.h"
#include "port/enhancements/events/hooks/Events.h"

void Rando::ObjectBehavior::InitPropBehavior() {
	COND_VB_SHOULD(VB_OVERRIDE_PROP_SPAWN, EVENT_PRIORITY_NORMAL, true, {
		s16* position = va_arg(args, s16*);

		// if (!IS_RANDO) {
        //     return;
        // }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(position[0], position[1], position[2]);

        if (!ShouldOverrideSpawn(randoCheckId)) {
            return;
        }

        int32_t spawnPosition[3];
        spawnPosition[0] = (int32_t)position[0];
        spawnPosition[1] = (int32_t)position[1];
        spawnPosition[2] = (int32_t)position[2];

        CustomObject::AddToSpawnQueue(randoCheckId, spawnPosition);
        *should = true;
	})

}
