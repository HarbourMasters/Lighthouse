#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "core1/ml.h"

extern f32 D_8037C5B0[3];
}

namespace {
constexpr s32 kDialogFlagSkippable = 0x1;

u16 sDialogShown[2] = { 0, 0 };
const ProximityDialogMap* sWorldMaps = nullptr;
s32 sWorldMapCount = 0;
} // namespace

void ProximityDialogs_Run(const ProximityDialogMap* maps, int count) {
    if (getGameMode() != GAME_MODE_3_NORMAL || gcdialog_hasCurrentTextId()) {
        return;
    }

    const ProximityDialogMap* entry = nullptr;
    const s32 map = gsworld_getMap();
    for (s32 i = 0; i < count; i++) {
        if (maps[i].map == map) {
            entry = &maps[i];
            break;
        }
    }
    if (entry == nullptr) {
        return;
    }
    if (entry->gate != nullptr && !entry->gate()) {
        return;
    }

    for (s32 i = 0; i < entry->pageCount; i++) {
        const ProximityDialogPage& page = entry->pages[i];
        const bool tokenIsGuard = page.doneBit == 0;
        if (tokenIsGuard ? mumboscore_get((enum mumbotoken_e)page.token)
                         : (sDialogShown[page.word] & page.doneBit) != 0) {
            continue;
        }
        if (page.skipIfJiggy > 0 && jiggyscore_isCollected((enum jiggy_e)page.skipIfJiggy)) {
            continue;
        }
        if (page.needFlag > 0 && !fileProgressFlag_get((enum file_progress_e)page.needFlag)) {
            continue;
        }
        if (page.needFlagClear > 0 && fileProgressFlag_get((enum file_progress_e)page.needFlagClear)) {
            continue;
        }
        f32 anchor[3] = { page.anchor[0], page.anchor[1], page.anchor[2] };
        if (!(ml_vec3f_distance(anchor, D_8037C5B0) < page.radius)) {
            return;
        }
        gcdialog_showDialog(page.textId, page.dialogFlags | kDialogFlagSkippable, (f32*)page.dialogPos, NULL, NULL,
                            NULL);
        sDialogShown[page.word] |= page.doneBit;
        if (page.token >= 0) {
            mumboscore_set((enum mumbotoken_e)page.token, true);
        }
    }
}

void ProximityDialogs_Enable(const ProximityDialogMap* maps, int count) {
    sWorldMaps = maps;
    sWorldMapCount = count;
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, sWorldMapCount > 0,
              [](IEvent*) { ProximityDialogs_Run(sWorldMaps, sWorldMapCount); });
}

bool ProximityDialogs_IsShown(int word, unsigned bits) {
    return (sDialogShown[word] & bits) != 0;
}

void ProximityDialogs_ClearShown(int word, unsigned bits) {
    sDialogShown[word] &= ~(u16)bits;
}
