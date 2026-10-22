#include <libultraship/libultraship.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Save/SaveManager.h"
#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"

#include "functions.h"
extern "C" {
#include "enums.h"
}

#define CVAR_HIGHLIGHT_HEARD_HINTS CVAR_ENHANCEMENT("Gameplay.HighlightHeardHints")

namespace {

constexpr s32 kFirstHintDialog = 0x1083;
constexpr s32 kHintCount = 30;
constexpr s32 kQuizShowAnswers = 2;
constexpr s32 kQuizFreeAnswers = 10;
constexpr s8 kRightAnswer = 1;

const void* sYellowBox;

} // namespace

// Always on, so turning the highlight on later covers hints heard before.
static void RegisterHeardHints_Init() {
    COND_HOOK(OnDialogLoaded, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnDialogLoaded*>(event);
        const s32 hint = ev->textId - kFirstHintDialog;
        ShipSaveData* data = SaveManager_GetActiveShipData();
        if (hint >= 0 && hint < kHintCount && data != nullptr) {
            data->heardBrentildaHints |= 1u << hint;
        }
    });
}

static void RegisterHighlightHeardHints_Init() {
    const bool enabled = CVarGetInteger(CVAR_HIGHLIGHT_HEARD_HINTS, 0);
    sYellowBox = nullptr;

    COND_HOOK(OnQuizStateChange, EVENT_PRIORITY_NORMAL, enabled, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnQuizStateChange*>(event);
        if (ev->state != kQuizShowAnswers && ev->state != kQuizFreeAnswers) {
            return;
        }
        sYellowBox = nullptr;
        ShipSaveData* data = SaveManager_GetActiveShipData();
        if (ev->state != kQuizShowAnswers || ev->questionType != FFQT_3_GRUNTY || data == nullptr ||
            !(data->heardBrentildaHints & (1u << ev->questionIndex))) {
            return;
        }
        for (s32 i = 1; i < 4; i++) {
            if (ev->answerValues[i] == kRightAnswer) {
                sYellowBox = ev->zoomboxes[i];
            }
        }
    });

    COND_VB_SHOULD(VB_ZOOMBOX_YELLOW_TEXT, EVENT_PRIORITY_NORMAL, enabled, {
        const void* zoombox = va_arg(args, void*);
        *should = zoombox != nullptr && zoombox == sYellowBox && gcquiz_isNotInInitialState();
    });
}

static RegisterShipInitFunc initHeardHints(RegisterHeardHints_Init);
static RegisterShipInitFunc initHighlightHeardHints(RegisterHighlightHeardHints_Init, { CVAR_HIGHLIGHT_HEARD_HINTS });
