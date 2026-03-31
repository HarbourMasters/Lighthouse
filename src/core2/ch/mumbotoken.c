#include <ultra64.h>
#include "functions.h"
#include "variables.h"

typedef struct {
    enum mumbotoken_e uid;
}ActorLocal_MumboToken;

void func_802E0B10(Actor *this);
enum mumbotoken_e func_802E0CB0(Actor *this);

/* .data */
 ActorInfo D_803685A0 = { 
    MARKER_39_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN, ASSET_41A_SPRITE_MUMBO_TOKEN,
    0, NULL, 
    func_802E0B10, actor_update_func_80326224, func_80325934,
    2000, 0, 0.0f, 0
};

/* .bss */
enum mumbotoken_e D_8037E610;

/* .code */
void chMumboToken_collect(ActorMarker *marker, ActorMarker *other_marker){
    Actor *this;

    this = marker_getActor(marker);
    mumboscore_set(func_802E0CB0(this), true);
    func_8030E760(0x401, 1.0f, 0x7fff);
    timedFunc_set_1(0.75f, (GenFunction_1)func_8035644C, FILEPROG_4_MUMBO_TOKEN_TEXT);
    fxSparkle_mumboToken(&marker->propPtr->x);
    item_inc(ITEM_1C_MUMBO_TOKEN);
    marker_despawn(marker);
    
}

enum mumbotoken_e func_802E0A90(Actor *this){
    s32 id;
    s32 pos[3];
    s32 ret;

    enum map_e map_id = map_get();
    pos[0] = (s32)this->position[0];
    pos[1] = (s32)this->position[1];
    pos[2] = (s32)this->position[2];
    id = func_80307164(pos);
    if(id < 0){
        return 0;
    }
    else{
        ret = func_80306DBC(id) - 199;
        // [port] Fix duplicate mumbo token IDs (from BanjoRecomp)
        // Token in MMM Inside Loggo shares ID 0x3D with another token
        if (CVarGetInteger(CVAR_ENHANCEMENT("Fixes.MumboTokenMMM"), 0)) {
            if (ret == 0x3D && pos[0] == 424 && pos[1] == 170 && pos[2] == 304 && map_id == MAP_8D_MMM_INSIDE_LOGGO) {
                ret = 0x74; // Swap to unused ID
            }
        }
        // Token in CCW Spring shares ID 0x5E with another token
        if (CVarGetInteger(CVAR_ENHANCEMENT("Fixes.MumboTokenCCW"), 0)) {
            if (ret == 0x5E && pos[0] == -2649 && pos[1] == 0 && pos[2] == -395 && map_id == MAP_43_CCW_SPRING) {
                ret = 0x5D; // Swap to unused ID
            }
        }
        return ret;
    }
}

void func_802E0B10(Actor *this){
    ActorLocal_MumboToken *local;
    f32 sp28[3];

    local = (ActorLocal_MumboToken *)&this->local;
    if(!this->initialized){
        this->initialized = true;
        // [port] GV Water Pyramid: lower token to ground level after water drains
        if (CVarGetInteger(CVAR_ENHANCEMENT("Fixes.MumboTokenGV"), 0)) {
            if (map_get() == MAP_15_GV_WATER_PYRAMID && jiggyscore_isCollected(JIGGY_42_GV_WATER_PYRAMID)) {
                this->position[1] = -1430.0f;
            }
        }
        if(local->uid == 0){
            if(!this->unk44_2){
                local->uid = D_8037E610;
            }
            else{
                local->uid = func_802E0A90(this);
            }
        }
        if( mumboscore_get(local->uid)
            || volatileFlag_get(VOLATILE_FLAG_1)
            || volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME)
            || volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE)
        ){
            marker_despawn(this->marker);
            return;
        }
    }//L802E0BD4

    if(!func_8032BBE8(this)){
        marker_setCollisionScripts(this->marker, chMumboToken_collect, NULL, NULL);
    }

    if(this->marker->unk14_21 && randf() < 0.1){
        sp28[0] = this->position[0] + randf2(-30.0f, 30.0f);
        sp28[1] = this->position[1] + randf2(25.0f, 55.0f);
        sp28[2] = this->position[2] + randf2(-30.0f, 30.0f);
        func_803525A0(sp28);
    }
}

enum mumbotoken_e func_802E0CB0(Actor *this){
    ActorLocal_MumboToken *local;
    f32 sp28[3];

    local = (ActorLocal_MumboToken *)&this->local;
    return local->uid;
}

void func_802E0CB8(enum mumbotoken_e mumbo_token_id){
    D_8037E610 = mumbo_token_id;
}
