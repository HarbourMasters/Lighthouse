// BanjoDecomp: mudhut.c
#include "core2/statetimer.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include <bk_math.h>

#include "port/Patches/Patches.h"

/* TODO move declarations to respective headers*/
extern void player_stateTimer_set(enum state_timer_e, f32);
extern f32 player_stateTimer_get(enum state_timer_e);


Actor *spawnQueue_bundle_f32(u32,f32,f32,f32);
void subaddie_set_state(Actor *, u32);

/* local declarations */
Actor *chMudHut_draw(ActorMarker *this, Gfx** gdl, Mtx** mtx, Vtx **vtx);
void chMudHut_spawnExplosion(ActorMarker *);
void chMudHut_update(Actor *this);

/* .data section */
ActorAnimationInfo chMudHutAnimations[4] = {
    {0, 0.0f},
    {0, 0.0f},
    {ASSET_4E_ANIM_MUD_HUT_SMASH, 0.25f},
    {ASSET_4E_ANIM_MUD_HUT_SMASH, 1000000.0f}
};

enum bundle_e D_80390B50[6] = {
    BUNDLE_A_BGS_HUT_SHOCKSPRING_PAD,
    BUNDLE_A_BGS_HUT_SHOCKSPRING_PAD,
    BUNDLE_B_BGS_HUT_MUSIC_NOTE,
    BUNDLE_A_BGS_HUT_SHOCKSPRING_PAD,
    BUNDLE_A_BGS_HUT_SHOCKSPRING_PAD,
    BUNDLE_C_BGS_HUT_JIGGY
};

ActorInfo chMudHut = {MARKER_D5_BGS_MUD_HUT, ACTOR_C_MUD_HUT, ASSET_7D8_MODEL_MM_HUT_TOP, 0x01, chMudHutAnimations,
    chMudHut_update, actor_update_func_80326224, chMudHut_draw,
    0, 0, 0.0f, 0
};

/* .code section */
void chMudHut_makeWadingBootsRunOutInstantly(void){
    if((getGameMode() != GAME_MODE_7_ATTRACT_DEMO) && (1.5 < player_stateTimer_get(STATE_TIMER_2_LONGLEG)) ){
        player_stateTimer_set(STATE_TIMER_2_LONGLEG, 1.5);
    }
}

void chMudHut_checkBGSChecksums(void){
    // [port] anti-tamper: ROM CRC check via osPiReadIo — not applicable on PC
#if ANTI_TAMPER
    u32 sp1C;
    osPiReadIo(0xD10, &sp1C);
    if(sp1C = (u16)(sp1C-0x400)){
        chMudHut_makeWadingBootsRunOutInstantly();
    }
#endif
}

Actor *chMudHut_draw(ActorMarker *this, Gfx** gdl, Mtx** mtx, Vtx **vtx){
    Actor *thisActor;

    thisActor = marker_getActor(this);
    modelRender_setAppendageVisibility(1, thisActor->state == 1);
    if(thisActor->state == 3)
        return thisActor;
    
    return actor_draw(this, gdl, mtx, vtx);
}

void chMudHut_spawnExplosion(ActorMarker *this){
    Actor *thisActor;

    thisActor = marker_getActor(this);
    thisActor = actor_spawnWithYaw_f32(ACTOR_D_WOOD_DEMOLISHED, thisActor->position, 0);
    thisActor = actor_spawnWithYaw_f32(ACTOR_4D_STEAM_2, thisActor->position, 0);
    if(this);
}

static void chMudHut_dropRecordedBundle(Actor *this, s32 tmp, s32 fullBundle){
    f32 pos[3];
    if(tmp < 0 || tmp >= 5){
        return; // jiggy / out of range: handled by JIGGY_SPAWN, not the hut
    }
    if(!fullBundle && tmp == 2){
        return; // note: spawn live but skip on reload
    }
    pos[0] = this->position_x;
    pos[1] = this->position_y + 130.0f;
    pos[2] = this->position_z;
    __spawnQueue_add_4((GenFunction_4) spawnQueue_bundle_f32, D_80390B50[tmp], reinterpret_cast(s32, pos[0]), reinterpret_cast(s32, pos[1]), reinterpret_cast(s32, pos[2]));
}

static void chMudHut_replaySmash(Actor *this, s32 tmp){
    sfx_playFadeShorthandDefault(SFX_5B_HEAVY_STUFF_FALLING, 1.0f, 28000, this->position, 0x12C, 0xBB8);
    subaddie_set_state(this, 2);
    this->marker->propPtr->unk8_3 = 0;
    actor_playAnimationOnce(this);
    if(tmp == 5){
        coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 28000);
    }
    __spawnQueue_add_1((GenFunction_1)chMudHut_spawnExplosion, (uintptr_t)this->marker);
    chMudHut_dropRecordedBundle(this, tmp, 1);
}

void chMudHut_update(Actor *this){

    f32 diffPos[3];
    f32 plyrPos[3];
    s32 tmp;

    if(gsworld_getUnk0() == 2){
        if(!this->initialized){
            this->marker->collidable = false;
            this->initialized = true;
            // Anchor: already smashed this session — restore broken, re-drop non-tracked loot.
            tmp = port_hutSmash_get((s32)this->position_x, (s32)this->position_y, (s32)this->position_z);
            if(tmp >= 0){
                this->state = 3;
                this->marker->propPtr->unk8_3 = 0;
                chMudHut_dropRecordedBundle(this, tmp, 0);
                return;
            }
        }

        switch(this->state){
            case 1:
                this->marker->propPtr->unk8_3 = 1;
                player_getPosition(plyrPos);
                diffPos[0] = plyrPos[0] - this->position_x;
                diffPos[1] = plyrPos[1] - this->position_y;
                diffPos[2] = plyrPos[2] - this->position_z;
                if( (150.0f < diffPos[1])
                    && (player_getActiveHitbox(this->marker) == HITBOX_1_BEAK_BUSTER)
                    && (player_isStableWithExtraSteps())
                    && (LENGTH_VEC3F(diffPos) < 350.f)
                ){
                    tmp = (s32)( (this->position_y - 600.f)/430.0f);
                    diffPos[0] = this->position_x;
                    diffPos[1] = this->position_y;
                    diffPos[2] = this->position_z;
                    diffPos[1] += 130.0;

                    sfx_playFadeShorthandDefault(SFX_5B_HEAVY_STUFF_FALLING, 1.0f, 28000, this->position, 0x12C, 0xBB8);
                    subaddie_set_state(this, 2);
                    this->marker->propPtr->unk8_3 = 0;
                    actor_playAnimationOnce(this);
                    if(tmp == 5){
                        coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 28000);
                    }
                    __spawnQueue_add_1((GenFunction_1)chMudHut_spawnExplosion, (uintptr_t)this->marker);

                    if (tmp < 5) {
                        __spawnQueue_add_4((GenFunction_4) spawnQueue_bundle_f32, D_80390B50[tmp], reinterpret_cast(s32, diffPos[0]), reinterpret_cast(s32, diffPos[1]), reinterpret_cast(s32, diffPos[2]));
                    }
                    else {
                        jiggy_spawn(JIGGY_23_BGS_HUTS, diffPos);
                    }
                    port_hutSmash_record((s32)this->position_x, (s32)this->position_y, (s32)this->position_z, tmp);
                }
                else {
                    tmp = port_hutSmash_get((s32)this->position_x, (s32)this->position_y, (s32)this->position_z);
                    if(tmp >= 0){
                        chMudHut_replaySmash(this, tmp);
                    }
                }
                break;
            case 2:
                this->marker->propPtr->unk8_3 = 0;
                if(0.99 < anctrl_getAnimTimer(this->anctrl)){
                    this->state = 3;
                }
                break;
            case 3:
                this->marker->propPtr->unk8_3 = 0;
                break;
        }
    }
    else{

    }
}
