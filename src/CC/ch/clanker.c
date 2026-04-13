// BanjoDecomp: code_3400.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

typedef struct {
    u8 *unk0;
    f32 unk4;
    s32 unk8;
} ActorLocal_CC_3400;

void chCCGrate_update(Actor *this);
/* .data */
u8 D_80389E40[] = {3, 3, 2, 0};

ActorInfo D_80389E44 = { 
    0x1A9, 0x28C, 0x435, 
    0, NULL, 
    chCCGrate_update, NULL, actor_draw, 
    0, 0, 0.0f, 0
};

ActorInfo D_80389E68 = { 
    0x1A9, 0x28D, 0x436, 
    0, NULL, 
    chCCGrate_update, NULL, actor_draw, 
    0, 0, 0.0f, 0
};

ActorInfo D_80389E8C = { 
    0x1A9, 0x28E, 0x437, 
    0, NULL, 
    chCCGrate_update, NULL, actor_draw, 
    0, 0, 0.0f, 0
};


/* .code */
void chCCGrate_setNextState(Actor *this, s32 next_state){
    ActorLocal_CC_3400 *local = (ActorLocal_CC_3400 *) &this->local;

    if(next_state == 3)
        FUNC_8030E624(SFX_1E_HITTING_AN_ENEMY_2, 1.0f, 25000);

    if(next_state == 2){
        coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, -1);
        FUNC_8030E624(SFX_3F6_RUBBING, 0.9f, 29000);
        local->unk4 = 0.0f;
    }

    if(next_state == 4){
        marker_despawn(this->marker);
    }

    this->state = next_state;
}

void chCCGrate_ow2(ActorMarker *marker, ActorMarker *other_marker){
    coMusicPlayer_playMusic(COMUSIC_2B_DING_B, -1);
}

void chCCGrate_die(ActorMarker *marker, ActorMarker *other_marker){
    Actor *actor = marker_getActor(marker);
    ActorLocal_CC_3400 *local = (ActorLocal_CC_3400 *) &actor->local;

    if(actor->state == 1){
        chCCGrate_setNextState(actor, *local->unk0);
    }
}

void chCCGrate_update(Actor * this){
    ActorLocal_CC_3400 *local = (ActorLocal_CC_3400 *) &this->local;
    f32 sp20 = time_getDelta();
    
    if(!this->volatile_initialized){
        this->volatile_initialized = true;
        this->marker->propPtr->unk8_3 = 1;
        local->unk0 = &D_80389E40[this->modelCacheIndex - 0x28C];
        local->unk8 = 0;
        marker_setCollisionScripts(this->marker, NULL, chCCGrate_ow2, chCCGrate_die);
        chCCGrate_setNextState(this, 1);
        if(this->modelCacheIndex == 0x28E && jiggyscore_isSpawned(JIGGY_18_CC_BOLT)){
            marker_despawn(this->marker);
        }
        return;
    }//L803899D4

    if(this->state == 1){
        if(local->unk8){
            chCCGrate_setNextState(this, *local->unk0);
        }
    }//L80389A10

    if(this->state == 3){
        chCCGrate_setNextState(this, 4);
    }

    if(this->state == 2){
        local->unk4 += 250.0f*sp20;
        this->position_y += 250.0f*sp20;
        if(250.0f <= local->unk4){
            chCCGrate_setNextState(this, 4);
        }
    }
}
