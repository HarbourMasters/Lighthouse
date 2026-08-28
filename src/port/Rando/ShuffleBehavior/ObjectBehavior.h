#ifndef RANDO_OBJECT_BEHAVIOR_H
#define RANDO_OBJECT_BEHAVIOR_H

#include "port/Rando/Rando.h"
#include "enums.h"
#include <vector>

bool CheckEnemyOverlapPosition(int32_t pos[3]);
int32_t GetJinjoActorMarkerId(actor_e actorId);
extern std::vector<RandoCheckId> enemyKillOverlapList;

namespace Rando {

namespace ObjectBehavior {

void Init();

} // namespace ObjectBehavior

} // namespace Rando

#endif // RANDO_OBJECT_BEHAVIOR_H
