#ifndef RANDO_OBJECT_BEHAVIOR_H
#define RANDO_OBJECT_BEHAVIOR_H

#include "port/Rando/Rando.h"
#include <vector>

bool CheckEnemyOverlapPosition(int32_t pos[3]);
int32_t GetJinjoActorMarkerId(actor_e actorId);
extern std::vector<RandoCheckId> enemyKillOverlapList;

namespace Rando {

namespace ObjectBehavior {

void Init();
void InitBundleBehavior();
void InitJiggyBehavior();
void InitJinjoBehavior();
void InitMolehillBehavior();
void InitMusicNoteBehavior();
void InitPropBehavior();
void InitStopNSwopBehavior();

void ModifySwitchBehavior(int32_t switchActorId);
void ModifyGobiBehavior(void* gobiActor);
void ModifyBoggyBehavior(void* boggyActor);
void ModifyPresentBehavior(void* presentActor);
void ModifyStopNSwopWorldBehavior(void* snsActor);
void DespawnCollectedBundles();

} // namespace ObjectBehavior

} // namespace Rando

#endif // RANDO_OBJECT_BEHAVIOR_H
