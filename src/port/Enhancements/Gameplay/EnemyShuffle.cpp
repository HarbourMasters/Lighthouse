#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include "actor.h"

#define CVAR_SHUFFLE_ENEMIES CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Enabled")

extern "C" {
Actor* actor_new(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
extern Vec3fArray* D_8036E568;

extern ActorInfo chBigbutt;
extern ActorInfo chTermite;
extern ActorInfo chgrublinInfo;
extern ActorInfo gChClam;
// extern ActorInfo chSnowman;
extern ActorInfo gChFlibbit;
extern ActorInfo D_80367130;
// extern ActorInfo gChVegetablesBawlB;
// extern ActorInfo gChVegetablesTopperB;
// extern ActorInfo gChVegetablesCollywobbleB;
extern ActorInfo D_8038F908;
extern ActorInfo D_80390DAC;
}

std::map<actor_e, const char*> shufflableEnemyMap = {
    { ACTOR_4_BIGBUTT, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Bigbutt") },
    { ACTOR_5_TERMITE, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Termite") },
    { ACTOR_6_GRUBLIN, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Grublin") },
    { ACTOR_69_CLAM, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.YumYum") },
    { ACTOR_133_FLIBBIT_RED, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.RedFlibbit") },
    { ACTOR_134_BUZZBOMB, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Buzzbomb") },
    { ACTOR_1E9_SNARE_BEAR, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Snarebear") },
    { ACTOR_30D_BOOM_BOX_FAST, CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.BoomBox") },
};

std::vector<actor_e> shufflableEnemyList;

// clang-format off
std::map<actor_e, std::pair<ActorInfo, int32_t>> enemyInfoMap = {
    { ACTOR_4_BIGBUTT,                          { chBigbutt, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_1 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_5_TERMITE,                          { chTermite, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_6_GRUBLIN,                          { chgrublinInfo, ACTOR_FLAG_UNKNOWN_25 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_0 } },
    { ACTOR_69_CLAM,                            { gChClam, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_25 } },
    // Crashes when leaving an area with this actor spawned
    // { ACTOR_124_SIR_SLUSH,                      { chSnowman, ACTOR_FLAG_UNKNOWN_2 } },
    { ACTOR_133_FLIBBIT_RED,                    { gChFlibbit, ACTOR_FLAG_UNKNOWN_25 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_0 } },
    { ACTOR_134_BUZZBOMB,                       { D_80367130, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_25 } },
    // Vegetables do not seem to move around the map outside of Spiral Mountain, need to bypass Flag checks
    // { ACTOR_BAWL_THE_ONION_B,                   { gChVegetablesBawlB, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_25 } },
    // { ACTOR_TOPPER_THE_CARROT_B,                { gChVegetablesTopperB, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_25 } },
    // { ACTOR_COLLYWOBBLE_B,                      { gChVegetablesCollywobbleB, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_21 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_1E9_SNARE_BEAR,                     { D_8038F908, ACTOR_FLAG_UNKNOWN_3 } },
    { ACTOR_30D_BOOM_BOX_FAST,                  { D_80390DAC, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_25 } },
};
// clang-format on

bool IsEnemyWhitelisted(actor_e enemyId) {
    for (auto& enemy : shufflableEnemyList) {
        if (enemy == enemyId) {
            return true;
        }
    }

    return false;
}

void UpdateEnemyShuffleList() {
    shufflableEnemyList.clear();
    if (CVAR_SHUFFLE_ENEMIES) {
        for (auto& [enemy, cvar] : shufflableEnemyMap) {
            if (CVarGetInteger(cvar, 0)) {
                shufflableEnemyList.push_back(enemy);
            }
        }
    }
}

void RegisterEnemyShuffle_Init() {
    UpdateEnemyShuffleList();

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, CVAR_SHUFFLE_ENEMIES, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        int32_t spawnPosition[3];
        spawnPosition[0] = ev->posX;
        spawnPosition[1] = ev->posY;
        spawnPosition[2] = ev->posZ;

        if (IsEnemyWhitelisted((actor_e)ev->actorId)) {
            event->Cancelled = true;

            actor_e shuffledActorId = shufflableEnemyList[rand() % shufflableEnemyList.size()];
            Actor* newActor = actor_new(spawnPosition, ev->rot, &enemyInfoMap.at(shuffledActorId).first,
                                        enemyInfoMap.at(shuffledActorId).second);

            if (shuffledActorId == ACTOR_124_SIR_SLUSH) {
                newActor->marker->unk44 = D_8036E568;
            }

            ev->result = newActor;
        }
    })
    COND_HOOK(OnEnemyShuffleSelectionChanged, EVENT_PRIORITY_NORMAL, CVAR_SHUFFLE_ENEMIES, [](IEvent* event) {
        OnEnemyShuffleSelectionChanged* ev = (OnEnemyShuffleSelectionChanged*)event;

        UpdateEnemyShuffleList();
    })
}

static RegisterShipInitFunc initEnemyShuffleFunc(RegisterEnemyShuffle_Init, { CVAR_SHUFFLE_ENEMIES });