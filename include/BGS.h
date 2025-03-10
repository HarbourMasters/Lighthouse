#ifndef BGS_H
#define BGS_H

#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

Actor *mrvile_draw(ActorMarker *, Gfx **, Mtx **, Vtx **);
void mrvile_update(Actor *);
void mrvile_updateHealth(ActorMarker *);
void mrvile_adjustPosition(f32 arg0[3], f32 arg1[3], f32 arg2, f32 arg3, s32 arg4);
void mrvile_setAnimation(Actor *this, s32 arg1);
void mrvile_initialize(Actor *this);
void mrvile_updatePosition(Actor *this);
void mrvile_setState(Actor *this, s32 next_state);
f32 *mrvile_getPostion(ActorMarker *marker);
bool mrvile_isPlayerClose(ActorMarker *marker);
bool mrvile_isInState1(ActorMarker *marker);
void mrvile_free(Actor *this);
void chvile_setState4(ActorMarker *marker);
void chvile_setState2(ActorMarker *marker);
void chvile_setState3(ActorMarker *marker);
void chvile_setState5(ActorMarker *marker);
void chvile_setState6(ActorMarker *marker);
void chvile_setState1(ActorMarker *arg0);
BKModelBin *chvilegame_get_grumblie_model(ActorMarker *marker);
void handle_tanktup_leg_collision(Actor *this);
bool chyumblie_is_edible(ActorMarker *arg0);
bool chyumblie_onEaten(ActorMarker *arg0);

#ifdef __cplusplus
}
#endif


#endif // BGS_H
