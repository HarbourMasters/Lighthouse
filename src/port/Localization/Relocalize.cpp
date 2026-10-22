#include <libultraship/libultraship.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ResourceHelpers.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "core2/gc/dialog.h"
#include "core2/gc/zoombox.h"
void clearDialogStrings(void);
void loadDialogStrings(s32 text_id);
void dialog_setState(DialogState next_state);
void* assetcache_get(s32 assetId);
s32 code94620_func_8031B5B0(void);
}

namespace {
int sDialogGen = -1;
int sQuizGen = -1;

void SwapBoxText(GcZoombox* zoombox, s32 count, char** strings) {
    const bool reported = zoombox != nullptr && zoombox->unk134 == 3;
    if (!func_8031877C(zoombox) || !gczoombox_setStrings(zoombox, count, strings) || !reported) {
        return;
    }
    zoombox->unk13A &= ~4;
    zoombox->unk1A4_25 = 0;
    zoombox->state = 1;
}

bool IsStructural(const BKDialog& entry) {
    return entry.cmd < 0 && entry.cmd != -8 && entry.cmd != -9;
}

void RelocalizeDialog(OnDialogUpdate* ev) {
    s32 passed[2] = {};
    for (s32 i = 0; i < 2; i++) {
        for (s32 j = 0; j < ev->stringIndices[i] && j < ev->stringCounts[i]; j++) {
            passed[i] += IsStructural(ev->dialogs[i][j]);
        }
    }
    clearDialogStrings();
    loadDialogStrings(ev->textId);
    for (s32 i = 0; i < 2; i++) {
        s32 j = 0;
        for (s32 n = 0; j < ev->stringCounts[i] && n < passed[i]; j++) {
            n += IsStructural(ev->dialogs[i][j]);
        }
        ev->stringIndices[i] = j;
        if (j < ev->stringCounts[i]) {
            ev->stringCmds[i] = ev->dialogs[i][j].cmd;
            ev->strings[i] = (char*)ev->dialogs[i][j].str;
        }
    }
    *ev->pageState = 0;
    if (ev->state == DIALOG_STATE_3 && func_8031877C((GcZoombox*)ev->zoomboxes[ev->activeZoombox])) {
        dialog_setState(DIALOG_STATE_2);
    }
}

void RelocalizeQuiz(OnQuizUpdate* ev) {
    s32 slotCmd[4];
    for (s32 i = 0; i < 4; i++) {
        slotCmd[i] = ev->answerStrCounts[i] ? ((u8*)ev->answerOptions[i * 4])[-2] : -1;
        ev->answerStrCounts[i] = 0;
    }
    *ev->questionBin = assetcache_get(ev->assetId);
    u8* bin = (u8*)*ev->questionBin;
    u8* slot = bin + 3 + 2 * ResourceMgr_GetDialogSlot(ev->assetId, code94620_func_8031B5B0());
    u8* entry = bin + (slot[0] | (slot[1] << 8));
    const s32 count = *entry++;
    for (s32 j = 0; j < count; j++) {
        const s32 cmd = *entry++;
        const s32 len = *entry++;
        for (s32 i = 0; i < 4; i++) {
            if (cmd == slotCmd[i] && ev->answerStrCounts[i] < 4) {
                ev->answerOptions[i * 4 + ev->answerStrCounts[i]++] = (uintptr_t)entry;
            }
        }
        entry += len;
    }
    for (s32 i = 0; i < 4; i++) {
        const s32 value = i == 0 ? 0 : ev->answerValues[i];
        if (value >= 0) {
            SwapBoxText((GcZoombox*)ev->zoomboxes[i], ev->answerStrCounts[value],
                        (char**)&ev->answerOptions[value * 4]);
        }
    }
}

} // namespace

static void RegisterRelocalize_Init() {
    REGISTER_LISTENER(OnDialogUpdate, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnDialogUpdate*>(event);
        const int gen = ResourceMgr_GetLanguageGeneration();
        if (gen == sDialogGen) {
            return;
        }
        const bool changed = sDialogGen != -1;
        sDialogGen = gen;
        if (changed && ev->textId != -1 && ev->dialogs[0] != nullptr) {
            RelocalizeDialog(ev);
        }
    });

    REGISTER_LISTENER(OnQuizUpdate, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnQuizUpdate*>(event);
        const int gen = ResourceMgr_GetLanguageGeneration();
        if (gen == sQuizGen) {
            return;
        }
        const bool changed = sQuizGen != -1;
        sQuizGen = gen;
        if (changed && ev->state != 0 && ev->questionType != -1) {
            RelocalizeQuiz(ev);
        }
    });

    REGISTER_LISTENER(OnQuizStateChange, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (reinterpret_cast<OnQuizStateChange*>(event)->state == 1) {
            sQuizGen = ResourceMgr_GetLanguageGeneration();
        }
    });
}

static RegisterShipInitFunc initRelocalize(RegisterRelocalize_Init);
