// BanjoDecomp: CC/ch/ccgrates.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

extern void port_breakable_recordBreak(s32 markerId, s32 x, s32 y, s32 z);
extern s32 port_breakable_isBroken(s32 map, s32 markerId, s32 x, s32 y, s32 z);

typedef struct {
    u8 *unk0;
    f32 unk4;
    s32 unk8;
} ActorLocal_CC_3400;

void chCCGrate_update(Actor *this);

/* .data */
enum chCCGrates_state_e {
    CC_GRATES_STATE_1_IDLE = 1,
    CC_GRATES_STATE_2_UNK,
    CC_GRATES_STATE_3_UNK,
    CC_GRATES_STATE_4_DESPAWN,
};

u8 D_80389E40[] = {3, 3, 2, 0};

ActorInfo chCCGrate1 = { 
    MARKER_1A9_CC_BREAKABLE_GRATES, 0x28C, ASSET_435_MODEL_CC_GREEN_GRATE,
    0, NULL, 
    chCCGrate_update, NULL, actor_draw, 
    0, 0, 0.0f, 0
};

ActorInfo chCCGrate2 = { 
    MARKER_1A9_CC_BREAKABLE_GRATES, 0x28D, ASSET_436_MODEL_CC_TEAL_GRATE,
    0, NULL, 
    chCCGrate_update, NULL, actor_draw, 
    0, 0, 0.0f, 0
};

ActorInfo chCCGrate3 = { 
    MARKER_1A9_CC_BREAKABLE_GRATES, 0x28E, 0x437,
    0, NULL, 
    chCCGrate_update, NULL, actor_draw, 
    0, 0, 0.0f, 0
};

/* .code */
void chCCGrate_setNextState(Actor *this, s32 next_state){
    ActorLocal_CC_3400 *local = (ActorLocal_CC_3400 *) &this->local;

    if(next_state == CC_GRATES_STATE_3_UNK)
        FUNC_8030E624(SFX_1E_HITTING_AN_ENEMY_2, 1.0f, 25000);

    if(next_state == CC_GRATES_STATE_2_UNK){
        coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, -1);
        FUNC_8030E624(SFX_3F6_RUBBING, 0.9f, 29000);
        local->unk4 = 0.0f;
    }

    if(next_state == CC_GRATES_STATE_4_DESPAWN){
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

    if(actor->state == CC_GRATES_STATE_1_IDLE){
        chCCGrate_setNextState(actor, *local->unk0);
    port_breakable_recordBreak((s32)actor->marker->id, (s32)actor->position[0],
                                   (s32)actor->position[1], (s32)actor->position[2]);
    }
}

void chCCGrate_update(Actor * this){
    ActorLocal_CC_3400 *local = (ActorLocal_CC_3400 *) &this->local;
    f32 sp20 = time_getDelta();
    
    if(!this->volatile_initialized){
        this->volatile_initialized = TRUE;
        this->marker->propPtr->unk8_3 = 1;
        local->unk0 = &D_80389E40[this->modelCacheIndex - 0x28C];
        local->unk8 = 0;
        marker_setCollisionScripts(this->marker, NULL, chCCGrate_ow2, chCCGrate_die);
        chCCGrate_setNextState(this, CC_GRATES_STATE_1_IDLE);
        if(this->modelCacheIndex == 0x28E && jiggyscore_isSpawned(JIGGY_18_CC_BOLT)){
            marker_despawn(this->marker);
        }
        else if(port_breakable_isBroken((s32)gsworld_getMap(), (s32)this->marker->id,
        (s32)this->position[0], (s32)this->position[1],
        (s32)this->position[2])){
            marker_despawn(this->marker);
        }
        return;
    }//L803899D4

    if(this->state == CC_GRATES_STATE_1_IDLE){
        if(local->unk8|| port_breakable_isBroken((s32)gsworld_getMap(), (s32)this->marker->id,
        (s32)this->position[0], (s32)this->position[1],
        (s32)this->position[2])){
            chCCGrate_setNextState(this, *local->unk0);
        }
    }//L80389A10

    if(this->state == CC_GRATES_STATE_3_UNK){
        chCCGrate_setNextState(this, CC_GRATES_STATE_4_DESPAWN);
    }

    if(this->state == CC_GRATES_STATE_2_UNK){
        local->unk4 += 250.0f*sp20;
        this->position_y += 250.0f*sp20;
        if(250.0f <= local->unk4){
            chCCGrate_setNextState(this, CC_GRATES_STATE_4_DESPAWN);
        }
    }
}
