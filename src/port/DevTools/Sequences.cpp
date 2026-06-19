// Sequences
//
// Dev-menu tools: jump straight into an attract demo or character parade, and warp into the
// Gruntilda battle / drop it into any phase with a full loadout. A button records a pending
// request that a GameFrameUpdate hook applies on the game thread.

#include "Sequences.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "prop.h"

// Sequences
void gcparade_beginFFParade(void);
void gcparade_beginFinalParade(void);
void func_8034BA7C(enum map_e map_id, int exit_id);
void func_8034B968(void);
extern int D_80386110;
void func_8025A55C(int, int, int);
void func_8025AB00(void);
enum level_e level_get(void);
int gctransition_8030BDC0(void);
int getGameMode(void);
void func_80324DBC(float time, int text_id, int arg2, float* position, void* caller, void* cb1, void* cb2);
void timedFunc_set_1(float time, void (*func)(int), int arg);
void func_80311714(int next_state);

// Final boss — warp & phase entry
void func_8031D04C(int map, int exit);
int gsworld_getMap(void);
bool player_is_present(void);
Actor* actorArray_findActorFromActorId(int actorId);
void chfinalboss_setPhase(ActorMarker* marker, int phase);
void code_7060_setVoidOutLocation(int map_id, int exit_id);
void func_8028F784(int lock);
void func_80324E38(float time, int mode);

// Final boss — loadout grant
void ability_setAllLearned(int val);
void ability_setAllUsed(int val);
int item_getCount(int item);
void item_adjustByDiffWithoutHud(int item, int diff);
void fileProgressFlag_set(int index, int set);

// Phase 3
void chfinalboss_phase2_endTextCallback(ActorMarker* marker, int text_id, int arg2);
int gcdialog_showDialog(int text_id, int arg1, float* pos, ActorMarker* marker,
                        void (*cb)(ActorMarker*, int, int), void (*endCb)(ActorMarker*, int, int));
extern ActorMarker* __chFinalBossFlightPadMarker;

// Phase 4
void chfinalboss_phase3_setState(Actor* actor, int state);

// Shield
void chfinalboss_spawnSpellBarrier(ActorMarker* arg0);
extern unsigned char __chFinalBossSpellBarrierActive;

// Phase 5
void chfinalboss_phase4_setState(Actor* actor, int state);
}

namespace Lighthouse {
namespace DevTools {

// Sequences

static int sPending = SEQ_NONE;

void RequestSequence(int seq) {
    sPending = seq;
}

static void TickSequences() {
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
        default:
            D_80386110 = seq - SEQ_ATTRACT_BASE;
            func_8034B968();
            break;
    }
}

// Final Boss

static const int PHASE_3_FLIGHT = 3;
static const int PHASE_4_JINJOS = 4;
static const int PHASE_5_JINJONATOR = 5;
static const int FB_STATE_1A_HANDOFF = 0x1A;
static const int FB_STATE_21_FALL = 0x21;
static const int FB_LOCAL_UNK3 = 0x3;
static const int FB_LOCAL_MIRROR_PHASE5 = 0x7;
static const int FB_LOCAL_UNKA = 0xA;
static const int GRANT_REAPPLY_FRAMES = 60;
static const int SUPPRESS_DIALOG_FRAMES = 180;

static bool sPendingWarp = false;
static int sGrantFrames = 0;
static int sPendingPhase = -1;
static int sSuppressDialogFrames = 0;
static bool sFlightPadRequested = false;
static bool sShieldRequested = false;

// Set/refill an item without popping the HUD
static void SetItemQuiet(int item, int target) {
    item_adjustByDiffWithoutHud(item, target - item_getCount(item));
}
static void MaxItemQuiet(int item) {
    item_adjustByDiffWithoutHud(item, 200);
}

static void GrantFullLoadout() {
    ability_setAllLearned(-1);
    ability_setAllUsed(-1);
    fileProgressFlag_set(FILEPROG_B9_DOUBLE_HEALTH, 1);
    fileProgressFlag_set(FILEPROG_BE_CHEATO_BLUEEGGS, 1);
    fileProgressFlag_set(FILEPROG_BF_CHEATO_REDFEATHERS, 1);
    fileProgressFlag_set(FILEPROG_C0_CHEATO_GOLDFEATHERS, 1);
    SetItemQuiet(ITEM_15_HEALTH_TOTAL, 16);
    SetItemQuiet(ITEM_14_HEALTH, 16);
    MaxItemQuiet(ITEM_D_EGGS);
    MaxItemQuiet(ITEM_F_RED_FEATHER);
    MaxItemQuiet(ITEM_10_GOLD_FEATHER);
}

// Reposition Grunty
static void SnapToHome(Actor* boss) {
    boss->position[0] = boss->unk1C[0];
    boss->position[1] = boss->unk1C[1];
    boss->position[2] = boss->unk1C[2];
}

static bool ApplyPhase(int phase) {
    Actor* boss = actorArray_findActorFromActorId(ACTOR_38B_GRUNTILDA_FINAL_BOSS);
    if (boss == nullptr || !boss->volatile_initialized) {
        return false;
    }

    // Enable input, camera control, and set voidout location
    func_8028F784(0);
    func_80324E38(0.0f, 0);
    code_7060_setVoidOutLocation(MAP_90_GL_BATTLEMENTS, WARP_GL_TOWER_5_ENTRANCE);

    switch (phase) {
        case PHASE_3_FLIGHT:
            chfinalboss_setPhase(boss->marker, PHASE_3_FLIGHT);
            SnapToHome(boss);
            if (__chFinalBossFlightPadMarker == nullptr && !sFlightPadRequested) {
                gcdialog_showDialog(0x111A, 4, nullptr, boss->marker, nullptr, chfinalboss_phase2_endTextCallback);
                sFlightPadRequested = true;
            }
            break;
        case PHASE_4_JINJOS:
            boss->local[FB_LOCAL_UNK3] = 2;
            boss->local[FB_LOCAL_UNKA] = 0;
            if (!__chFinalBossSpellBarrierActive && !sShieldRequested) {
                chfinalboss_phase3_setState(boss, FB_STATE_1A_HANDOFF);
                sShieldRequested = true;
            } else {
                chfinalboss_setPhase(boss->marker, PHASE_4_JINJOS);
            }
            break;
        case PHASE_5_JINJONATOR:
            chfinalboss_setPhase(boss->marker, PHASE_4_JINJOS);
            SnapToHome(boss);
            boss->local[FB_LOCAL_MIRROR_PHASE5] = 0;
            if (!__chFinalBossSpellBarrierActive) {
                chfinalboss_spawnSpellBarrier(boss->marker);
            }
            chfinalboss_phase4_setState(boss, FB_STATE_21_FALL);
            break;
        default:
            chfinalboss_setPhase(boss->marker, phase);
            SnapToHome(boss);
            break;
    }
    sSuppressDialogFrames = SUPPRESS_DIALOG_FRAMES;
    return true;
}

void RequestFinalBossPhase(int phase) {
    sPendingPhase = phase;
    sGrantFrames = GRANT_REAPPLY_FRAMES;
    if (gsworld_getMap() != MAP_90_GL_BATTLEMENTS) {
        sPendingWarp = true;
    }
}

static void TickFinalBoss() {
    if (sSuppressDialogFrames > 0) {
        sSuppressDialogFrames--;
    }

    if (sFlightPadRequested && __chFinalBossFlightPadMarker != nullptr) {
        sFlightPadRequested = false;
    }
    if (sShieldRequested && __chFinalBossSpellBarrierActive) {
        sShieldRequested = false;
    }

    if (sPendingWarp) {
        if (gctransition_8030BDC0()) {
            return;
        }
        func_8031D04C(MAP_90_GL_BATTLEMENTS, WARP_GL_TOWER_5_ENTRANCE);
        sPendingWarp = false;
        return;
    }

    if (gctransition_8030BDC0() || level_get() <= 0 || !player_is_present()) {
        return;
    }

    if (sGrantFrames > 0) {
        GrantFullLoadout();
        sGrantFrames--;
    }
    if (sPendingPhase >= 0 && ApplyPhase(sPendingPhase)) {
        sPendingPhase = -1;
    }
}

// Registration

void RegisterSequences_Init() {
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        TickSequences();
        TickFinalBoss();
    });

    REGISTER_VB_SHOULD(VB_FINALBOSS_ENTERING_DIALOG, EVENT_PRIORITY_NORMAL, {
        if (sPendingPhase >= 0 || sSuppressDialogFrames > 0) {
            *should = false;
        }
    });
}

} // namespace DevTools
} // namespace Lighthouse

static RegisterShipInitFunc sequencesInit(Lighthouse::DevTools::RegisterSequences_Init);
