#ifndef RANDO_OBJECT_BEHAVIOR_H
#define RANDO_OBJECT_BEHAVIOR_H

#include "port/Rando/Rando.h"

bool ShouldOverrideSpawn(RandoCheckId randoCheckId);
void LogOutSpawns(int32_t actorId, int16_t posX, int16_t posY, int16_t posZ);
int32_t GetJinjoActorMarkerId(actor_e actorId);

namespace Rando {

namespace ObjectBehavior {

void Init();
void InitBundleBehavior();
void InitJiggyBehavior();
void InitJinjoBehavior();
void InitMolehillBehavior();
void InitMusicNoteBehavior();
void InitPropBehavior();

} // namespace ObjectBehavior

} // namespace Rando

#endif // RANDO_OBJECT_BEHAVIOR_H
