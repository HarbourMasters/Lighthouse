#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/UI/enhancementTypes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include "spdlog/spdlog.h"

#include "actor.h"

#define CVAR_SHUFFLE_ENEMIES CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Enabled")

extern "C" {
Actor* __actor_spawnWithYaw_s32(enum actor_e arg0, s32 pos[3], s32 rot);
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
extern ActorInfo chMumMum;
extern ActorInfo chGruntlingRed;
extern ActorInfo chGruntlingBlue;
extern ActorInfo chGruntlingBlack;
extern ActorInfo D_80372D78;
}

std::vector<int32_t> shufflableEnemyList;

// clang-format off
std::map<int32_t, std::pair<std::string, const char*>> shufflableEnemyMap = {
    { ACTOR_4_BIGBUTT,              { "BigButt", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Bigbutt") } },
    { ACTOR_5_TERMITE,              { "Termite", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Termite") } },
    { ACTOR_6_GRUBLIN,              { "Grublin", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Grublin") } },
    { ACTOR_69_CLAM,                { "Yum-Yum", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.YumYum") } },
    // { ACTOR_124_SIR_SLUSH,          { "Sir Slush", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.SirSlush") } },
    { ACTOR_133_FLIBBIT_RED,        { "Red Flibbit", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.RedFlibbit") } },
    { ACTOR_134_BUZZBOMB,           { "Buzzbomb", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Buzzbomb") } },
    { ACTOR_1E9_SNARE_BEAR,         { "Snare Bear", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Snarebear") } },
    { ACTOR_30D_BOOM_BOX_FAST,      { "Boom Box", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.BoomBox") } },
    { ACTOR_34F_MUMMUM,             { "Mum-Mum", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Mummum") } },
    { ACTOR_367_GRUNTLING_RED,      { "Red Gruntling", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.RedGruntling") } },
    { ACTOR_3BF_GRUNTLING_BLUE,     { "Blue Gruntling", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.BlueGruntling") } },
    { ACTOR_3C0_GRUNTLING_BLACK,    { "Black Gruntling", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.BlackGruntling") } },
    { ACTOR_34E_LIMBO,              { "Limbo", CVAR_ENHANCEMENT("Gameplay.EnemyShuffle.Limbo") } },
};

std::map<int32_t, std::pair<ActorInfo, int32_t>> enemyInfoMap = {
    { ACTOR_4_BIGBUTT,              { chBigbutt, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_1 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_5_TERMITE,              { chTermite, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_6_GRUBLIN,              { chgrublinInfo, ACTOR_FLAG_UNKNOWN_25 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_0 } },
    { ACTOR_69_CLAM,                { gChClam, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_25 } },
    // Crashes when leaving an area with this actor spawned
    // { ACTOR_124_SIR_SLUSH,           { chSnowman, ACTOR_FLAG_UNKNOWN_2 } },
    { ACTOR_133_FLIBBIT_RED,        { gChFlibbit, ACTOR_FLAG_UNKNOWN_25 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_0 } },
    { ACTOR_134_BUZZBOMB,           { D_80367130, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_25 } },
    // Vegetables do not seem to move around the map outside of Spiral Mountain, need to bypass Flag checks
    // { ACTOR_BAWL_THE_ONION_B,       { gChVegetablesBawlB, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_25 } },
    // { ACTOR_TOPPER_THE_CARROT_B,    { gChVegetablesTopperB, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_25 } },
    // { ACTOR_COLLYWOBBLE_B,          { gChVegetablesCollywobbleB, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_21 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_1E9_SNARE_BEAR,         { D_8038F908, ACTOR_FLAG_UNKNOWN_3 } },
    { ACTOR_30D_BOOM_BOX_FAST,      { D_80390DAC, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_34F_MUMMUM,             { chMumMum, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_24 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_367_GRUNTLING_RED,      { chGruntlingRed, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_3BF_GRUNTLING_BLUE,     { chGruntlingBlue, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_3C0_GRUNTLING_BLACK,    { chGruntlingBlack, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_25 } },
    { ACTOR_34E_LIMBO,              { D_80372D78, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_24 | ACTOR_FLAG_UNKNOWN_25 } },
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
        for (auto& [enemyId, enemyData] : shufflableEnemyMap) {
            if (CVarGetInteger(enemyData.second, 0)) {
                shufflableEnemyList.push_back(enemyId);
            }
        }
    }
}

void RegisterEnemyShuffle_Init() {
    UpdateEnemyShuffleList();

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, CVAR_SHUFFLE_ENEMIES, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;
        Actor* newActor = NULL;

        int32_t spawnPosition[3];
        spawnPosition[0] = ev->posX;
        spawnPosition[1] = ev->posY;
        spawnPosition[2] = ev->posZ;

        if (IsEnemyWhitelisted((actor_e)ev->actorId)) {
            int32_t randomEnemy = rand() % shufflableEnemyList.size();
            event->Cancelled = true;

            actor_e shuffledActorId = (actor_e)shufflableEnemyList[randomEnemy];
            newActor = actor_new(spawnPosition, ev->rot, &enemyInfoMap.at(shuffledActorId).first,
                                 enemyInfoMap.at(shuffledActorId).second);

            // if (shuffledActorId == ACTOR_124_SIR_SLUSH) {
            //     newActor->marker->unk44 = D_8036E568;
            // }

            ev->result = newActor;
        }
    })
    COND_HOOK(OnEnemyShuffleSelectionChanged, EVENT_PRIORITY_NORMAL, CVAR_SHUFFLE_ENEMIES, [](IEvent* event) {
        OnEnemyShuffleSelectionChanged* ev = (OnEnemyShuffleSelectionChanged*)event;

        UpdateEnemyShuffleList();
    })
}

static RegisterShipInitFunc initEnemyShuffleFunc(RegisterEnemyShuffle_Init, { CVAR_SHUFFLE_ENEMIES });