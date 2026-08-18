// Dev Sequences
//
// Dev-menu buttons that jump straight to a character parade, the Banjo & Kazooie demo
// (GAME_MODE_9), or any attract demo. A button records a pending request that this
// GameFrameUpdate hook executes on the game thread.

#include "DevSequences.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include <spdlog/spdlog.h>

extern "C" {
#include "enums.h"
void gcparade_beginFFParade(void);
void gcparade_beginFinalParade(void);
void func_8034BA7C(enum map_e map_id, int exit_id); // warp (B&K demo uses MAP_1, exit 93)
void transitionToMap(enum map_e map, int exit, int transition);
void func_802E412C(int arg0, int arg1);
void func_8034B968(void);          // start the attract demo selected by D_80386110 (sets transition + D_80386114)
extern int D_80386110;             // attract-demo cycle index
void func_8025A55C(int, int, int); // fade the active music track
void func_8025AB00(void);          // finalize/stop the main music track
enum level_e level_get(void);
int gctransition_8030BDC0(void); // nonzero while a scene transition is active
int getGameMode(void);
void func_80324DBC(float time, int text_id, int arg2, float* position, void* caller, void* cb1, void* cb2);
void timedFunc_set_1(float time, void (*func)(int), int arg); // queue a 1-arg delayed call
void func_80311714(int next_state);                           // set g_Dialog.unk128_3 (parade-credit persist flag)
}

namespace Lighthouse {
namespace DevTools {

static int sPending = SEQ_NONE;
static int sPendingMap = -1;

void RequestSequence(int seq) {
    sPending = seq;
}

void RequestCutsceneMap(int mapId) {
    sPendingMap = mapId;
    sPending = SEQ_CUTSCENE_MAP;
}

void RegisterDevSequences_Init() {
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        if (sPending == SEQ_NONE || gctransition_8030BDC0()) {
            return;
        }

        if (sPending == SEQ_MODE9_DIALOG) {
            if (getGameMode() != GAME_MODE_9_BANJO_AND_KAZOOIE) {
                return;
            }
            timedFunc_set_1(1.0f, func_80311714, 0);
            func_80324DBC(1.0f, 0x11C9, 0xA0, nullptr, nullptr, nullptr, nullptr);
            timedFunc_set_1(1.0f, func_80311714, 1);
            sPending = SEQ_NONE;
            return;
        }

        if (level_get() <= 0) {
            return;
        }
        const int seq = sPending;
        sPending = SEQ_NONE;

        func_8025A55C(0, 0x1388, 0xB);
        func_8025AB00();

        switch (seq) {
            case SEQ_PARADE_FF:
                gcparade_beginFFParade();
                break;
            case SEQ_PARADE_FINAL:
                gcparade_beginFinalParade();
                break;
            case SEQ_MODE9_BK:
                func_8034BA7C(MAP_1_SM_SPIRAL_MOUNTAIN, 93);
                sPending = SEQ_MODE9_DIALOG;
                break;
            case SEQ_ENDING_ALL_100:
                transitionToMap(MAP_95_CS_END_ALL_100, 0, 1);
                break;
            case SEQ_GAME_OVER:
                func_802E412C(1, 0);
                transitionToMap(MAP_83_CS_GAME_OVER_MACHINE_ROOM, 0, 1);
                break;
            case SEQ_CUTSCENE_MAP:
                transitionToMap((enum map_e)sPendingMap, 0, 1);
                break;
            default:
                D_80386110 = seq - SEQ_ATTRACT_BASE;
                func_8034B968();
                break;
        }
    });
}

} // namespace DevTools
} // namespace Lighthouse

static RegisterShipInitFunc devSequencesInit(Lighthouse::DevTools::RegisterDevSequences_Init);
