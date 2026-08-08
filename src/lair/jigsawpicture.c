// BanjoDecomp: jigsawpicture.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "jigsawpicture.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/Network/Anchor/JigsawPedestal.h"
#include "port/Enhancements/Events/Hooks/Events.h"
extern void player_walkToPosition(f32[3], f32, void(*)(ActorMarker *), ActorMarker *);
extern void func_80324CFC(f32, enum comusic_e, s32);
extern void rand_seed(s32);
extern void func_8034DF30(Struct70s *, f32[3], f32[3], f32);
extern void updateStruct6DsOpacity(Struct70s *, s32, s32, f32);

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
}ActorLocal_lair_86F0;

typedef struct {
    u8 cost;
    u8 size_bits;
    u16 progress_flag; // enum file_progress_e
}Struct_lair_86F0_0;

void jigsawPicture_setState(Actor *this, s32 next_state);
void updateJigsawPictureActor(Actor *this);
Actor *jigsawPicture_draw(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx);

static s32 sBuzzedPedestalField = 0;

/* .data */
ActorInfo JIGSAW_PICTURE_ACTOR = { 0x1EB, 0x3B7, 0x48B, 0x1, NULL, updateJigsawPictureActor, actor_update_func_80326224, jigsawPicture_draw, 0, 0, 0.0f, 0};
ActorInfo JIGSAW_PICTURE_ACTOR_2 = { 0x1EB, 0x3BC, 0x538, 0x1, NULL, updateJigsawPictureActor, actor_update_func_80326224, jigsawPicture_draw, 0, 0, 0.0f, 0};
Struct_lair_86F0_0 D_803947F8[0xb] ={
    { 1, 0x1, FILEPROG_5D_MM_PUZZLE_PIECES_PLACED},
    { 2, 0x2, FILEPROG_5E_TTC_PUZZLE_PIECES_PLACED},
    { 5, 0x3, FILEPROG_60_CC_PUZZLE_PIECES_PLACED},
    { 7, 0x3, FILEPROG_63_BGS_PUZZLE_PIECES_PLACED},
    { 8, 0x4, FILEPROG_66_FP_PUZZLE_PIECES_PLACED},
    { 9, 0x4, FILEPROG_6A_GV_PUZZLE_PIECES_PLACED},
    {10, 0x4, FILEPROG_6E_MMM_PUZZLE_PIECES_PLACED},
    {12, 0x4, FILEPROG_72_RBB_PUZZLE_PIECES_PLACED},
    {15, 0x4, FILEPROG_76_CCW_PUZZLE_PIECES_PLACED},
    {25, 0x5, FILEPROG_7A_DOG_PUZZLE_PIECES_PLACED},
    { 4, 0x3, FILEPROG_7F_DOUBLE_HEALTH_PUZZLE_PIECES_PLACED}
};
s32 D_80394824[3] = {0xff, 0xff, 0};
ParticleScaleAndLifetimeRanges D_80394830 = {
    {0.17f, 0.24f},
    {0.08f, 0.13f},
    {0.0f, 0.01f},
    {0.9f, 0.9f},
    0.0f, 0.0f
};

/* .code */
s32 _puzzleCost(s32 index) {
    s32 override = port_getRomhackJiggyPuzzleCost(index);
    return (override >= 0) ? override : D_803947F8[index].cost;
}

s32 _puzzleSize(s32 index) {
    s32 override = port_getRomhackJiggyPuzzleSize(index);
    return (override >= 0) ? override : D_803947F8[index].size_bits;
}

s32 _puzzleFlag(s32 index) {
    s32 override = port_getRomhackJiggyPuzzleFlag(index);
    return (override >= 0) ? override : D_803947F8[index].progress_flag;
}

bool jigsawPicture_isJigsawPictureComplete(s32 arg0) {
    s32 cost = _puzzleCost(arg0 - 1);
    return fileProgressFlag_getN(_puzzleFlag(arg0 - 1), _puzzleSize(arg0 - 1)) == cost;
}

s32 getPictureCost(Actor *this){
    if (this->actorTypeSpecificField != 0 && this->actorTypeSpecificField < 0xC) {
        return _puzzleCost(this->actorTypeSpecificField - 1);
    }
    return 0;
}

bool isPictureComplete(Actor *this){
    ActorLocal_lair_86F0 *local;

    local = (ActorLocal_lair_86F0*)&this->local;
    return getPictureCost(this) == local->unk4;
}

s32 getLevelSpecificOpenFlag(Actor *this){
    return this->actorTypeSpecificField + 0x1B;
}

void activateDoubleHealth(void){
    func_802FAFD4(ITEM_14_HEALTH, 0x417);
    func_802FAFC0(ITEM_14_HEALTH, COMUSIC_2B_DING_B);
    fileProgressFlag_set(FILEPROG_B9_DOUBLE_HEALTH, true);
    func_80347958();
    item_adjustByDiffWithHud(ITEM_14_HEALTH, 0);
    gcpausemenu_80314AC8(1);
}

void afterPictureComplete(ActorMarker *marker) {
    Actor *this;
    u32 temp_t6;

    this = marker_getActor(reinterpret_cast(ActorMarker *, marker));
    if (this->actorTypeSpecificField < 0xA) {
        levelSpecificFlags_set(getLevelSpecificOpenFlag(this), true);
        return;
    }
    if (this->actorTypeSpecificField == 0xA) {
        func_8028F918(0);
        func_8028F918(2);
        levelSpecificFlags_set(LEVEL_FLAG_3F_LAIR_GRUNTY_DOOR_OPEN, true);
        return;
    }
    if (this->actorTypeSpecificField == 0xB) {
        timedFunc_set_0(1.5f, activateDoubleHealth);
        gcpausemenu_80314AC8(0);
    }
}

void onJigsawPodiumCollide(ActorMarker *marker, ActorMarker *other_marker){
    marker->isBanjoOnTop = true;
}

bool isBanjoOnPodium(ActorMarker *marker) {
    return player_isStableWithExtraSteps() && func_8028FB48(0x08000000) && marker->isBanjoOnTop;
}

s32 isPicturePiecePlaced(Actor *this, s32 arg1){
    ActorLocal_lair_86F0 *local;

    local = (ActorLocal_lair_86F0*)&this->local;
    return local->unk0 & (1 << arg1);
}

s32 jiggyPositionToID(Actor *this, s32 arg1){
    s32 phi_v1;
    switch (this->actorTypeSpecificField){
        case 7: 
            phi_v1 = (arg1 == 2) ? 0x1a4 : 0x190;
            break;

        case 3: 
            phi_v1 = 0x192;
            break;

        case 8: 
            phi_v1 = 0x19A;
            break;

        case 11: 
            phi_v1 = 0x1AE;
            break;
        default: 
            phi_v1 = 0x190;
            break;
    }
    return phi_v1 + arg1;
}

s32 getUnknownJigsawPictureIndex(Actor *this){
    switch (this->actorTypeSpecificField){
        case 3:
        case 8:
        case 0xb:
            return 0x1F;
    }
    return 0x1E;
}

void func_8038EDBC(Actor *this) {
    Struct70s *sp44;
    Struct70s *sp40;
    ActorLocal_lair_86F0 *local;
    s32 sp38;
    f32 sp34;
    f32 sp28[4];

    local = (ActorLocal_lair_86F0*)&this->local;
    sp38 = (this->modelCacheIndex == 0x3B7)? 0x190 : 0x192;
    sp44 = func_8034C2C4(this->marker, sp38);
    sp40 = func_8034C2C4(this->marker, sp38 + 1);
    if ((sp44 != 0) && (sp40 != 0) && (this->marker->unk14_21)) {
        sp28[0] = 1.0f;
        sp28[1] = 1.0f;
        sp28[2] = 1.0f;
        if (isBanjoOnPodium(this->marker) && local->unk8 < 0xFF) {
            local->unk8 = (local->unk8 + 8 < 0xFF) ? local->unk8 + 8 : 0xFF;
        }
        else if (!isBanjoOnPodium(this->marker) && (local->unk8 > 0)) {
            local->unk8 = (local->unk8 - 8 > 0) ? local->unk8 - 8 : 0;
        }
        sp34 = (0xFF - local->unk8) / 255.0;
        // [port] On N64, opaque render mode ignores vertex alpha — both meshes remain
        // visible regardless of alpha. On PC, alpha=0 hides the mesh entirely, causing
        // the golden podium platform (baked into the model) to disappear.
        // Keep sp44 (base mesh) always at alpha=1.0. Let sp40 (glow overlay)
        // crossfade normally.
        sp28[3] = 1.0f;
        func_8034DF30(sp44, sp28, sp28, 0);
        sp34 = 1.0 - sp34;
        sp28[3] = sp34;
        func_8034DF30(sp40, sp28, sp28, 0);
    }
}


void stoodOnPodiumCallback(ActorMarker *marker) {
    f32 sp24[3];
    Actor *this;

    this = marker_getActor(marker);
    vec3fArray_get_vec3f(func_803097A0(), getUnknownJigsawPictureIndex(this), sp24);
    func_8028E6EC(2);
    func_8028F918(0);
    func_8028F94C(4, sp24);
    jigsawPicture_setState(this, fileProgressFlag_get(FILEPROG_17_HAS_HAD_ENOUGH_JIGSAW_PIECES) ? 4 : 3);
}

void walkToPodium(Actor *this) {
    s32 pad3C;
    f32 sp30[3];
    f32 sp24[3];

    this->has_met_before = false;
    player_getPosition(sp30);
    sp24[0] = this->position[0];
    sp24[1] = this->position[1];
    sp24[2] = this->position[2];
    sp24[1] += 50.0f;
    player_walkToPosition(sp24, ml_vec3f_distance(sp30, sp24) / 150.0, stoodOnPodiumCallback, this->marker);
}

void bottlesInstructionsCallback(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    Actor *this;

    this = marker_getActor(marker);
    jigsawPicture_setState(this, (text_id == 0xf58) ? 1 : 4);
}

void gruntyLaughCallback(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    func_8030E6D4(SFX_EA_GRUNTY_LAUGH_1);
}

s32 getPicturePiecePosition(Actor *this) {
    ActorLocal_lair_86F0 *local;
    s32 phi_s0;
    s32 sp34;
    s32 phi_s2;

    phi_s0 = 0;
    local = (ActorLocal_lair_86F0*)&this->local;
    rand_seed(this->actorTypeSpecificField);
    if (this->actorTypeSpecificField >= 0xA) {
        for(phi_s2 = 0; phi_s2 < local->unk4; phi_s2++){
            sp34 = phi_s2;
            phi_s0 |= (1 << sp34);
        }
    } else {
        for(phi_s2 = 0; phi_s2 < local->unk4; phi_s2++){
            do{
                sp34 = randi2(0, getPictureCost(this));
            }
            while(1 << sp34 & phi_s0);
            phi_s0 |= 1 << sp34;
        }
    }
    return sp34;
}


void addOrRemovePieceFromDisplay(Actor *this, s32 arg1, bool arg2) {
    Struct70s *temp_v0;

    temp_v0 = func_8034C528(jiggyPositionToID(this, arg1));
    if (temp_v0 != 0) {
        updateStruct6DsOpacity(temp_v0, arg2 ? 0 : 0xff, arg2 ? 0xff : 0, 1.0f);
    }
}

void unlockAdditionalActions(Actor *this){
    ActorLocal_lair_86F0 *local;

    local = (ActorLocal_lair_86F0*)&this->local;
    if( (this->actorTypeSpecificField >= 2) 
        && (local->unk4 > 0) 
        && !isPictureComplete(this) 
        && !fileProgressFlag_get(FILEPROG_DF_CAN_REMOVE_ALL_PUZZLE_PIECES)
    ) {
        if (gcdialog_showDialog(0xF7C, 2, NULL, NULL, NULL, NULL)) {
            fileProgressFlag_set(FILEPROG_DF_CAN_REMOVE_ALL_PUZZLE_PIECES, true);
        }
    } else if ((this->actorTypeSpecificField >= 3) 
        && (local->unk4 >= 2) 
        && !isPictureComplete(this)
        && !fileProgressFlag_get(FILEPROG_E0_CAN_PLACE_ALL_PUZZLE_PIECES)
    ){
        if(gcdialog_showDialog(0xF7D, 2, NULL, NULL, NULL, NULL)) {
            fileProgressFlag_set(FILEPROG_E0_CAN_PLACE_ALL_PUZZLE_PIECES, true);
        }
    }
}

void jigsawPicture_setState(Actor *this, s32 next_state){
    ActorLocal_lair_86F0 *local;
    f32 sp50[3];
    s32 sp4C;
    s32 temp_s1;
    s32 phi_s0;

    local = (ActorLocal_lair_86F0*)&this->local;
    vec3fArray_get_vec3f(func_803097A0(), getUnknownJigsawPictureIndex(this), sp50);
    switch (next_state) {
        case 1: //L8038F3BC
            port_jigsawPedestal_release(this->actorTypeSpecificField);
            func_8028F918(0);
            break;

        case 2: //L8038F3CC
            walkToPodium(this);
            sfx_playFadeShorthandDefault(SFX_112_TINKER_ATTENTION, 1.0f, 32000, this->position, 500, 1000);
            break;

        case 3: //L8038F3F4
            func_803115C4(0xF7B);
            func_803115C4(0xF80);
            func_803115C4(0xF7F);
            if (item_getCount(ITEM_26_JIGGY_TOTAL) > 0) {
                gcdialog_showDialog(fileProgressFlag_get(FILEPROG_16_STOOD_ON_JIGSAW_PODIUM) ? 0xF5A : 0xF59, 6, sp50, this->marker, bottlesInstructionsCallback, NULL);
                fileProgressFlag_set(FILEPROG_17_HAS_HAD_ENOUGH_JIGSAW_PIECES, 1);
            } else {
                gcdialog_showDialog(0xF58, 6, sp50, this->marker, bottlesInstructionsCallback, NULL);
            }
            fileProgressFlag_set(FILEPROG_16_STOOD_ON_JIGSAW_PODIUM, 1);
            fileProgressFlag_set(FILEPROG_A7_NEAR_PUZZLE_PODIUM_TEXT, 1);
            break;

        case 8: //L8038F4AC
            if (local->unk4 > 0) {
                comusic_playTrack(SFX_REMOVE_JIGGY);
                this->lifetime_value = 1.0f;
                temp_s1 = getPicturePiecePosition(this);
                addOrRemovePieceFromDisplay(this, temp_s1, 0);
                local->unk4--;
                local->unk0 &= ~(1 << temp_s1);
                fileProgressFlag_setN(_puzzleFlag(this->actorTypeSpecificField - 1), local->unk4, _puzzleSize(this->actorTypeSpecificField - 1));
                item_adjustByDiffWithoutHud(ITEM_26_JIGGY_TOTAL, 1);
            }
            break;

        case 5: //L8038F550
            if (local->unk4 < getPictureCost(this)) {
                comusic_playTrack(COMUSIC_67_INSERTING_JIGGY);
                this->lifetime_value = 1.0f;
                local->unk4++;
                temp_s1 = getPicturePiecePosition(this);
                addOrRemovePieceFromDisplay(this, temp_s1, 1);
                local->unk0 |= (1 << temp_s1);
                fileProgressFlag_setN(_puzzleFlag(this->actorTypeSpecificField - 1), local->unk4, _puzzleSize(this->actorTypeSpecificField - 1));
                item_adjustByDiffWithoutHud(ITEM_26_JIGGY_TOTAL, -1);
                unlockAdditionalActions(this);
            }
            break;

        case 6: //L8038F604
            if (local->unk4 < getPictureCost(this)) {
                if(item_getCount(ITEM_26_JIGGY_TOTAL) > getPictureCost(this) - local->unk4){
                    sp4C = getPictureCost(this) - local->unk4;
                }
                else{
                    sp4C = item_getCount(ITEM_26_JIGGY_TOTAL);
                }
                comusic_playTrack(COMUSIC_67_INSERTING_JIGGY);
                this->lifetime_value = 1.0f;
                for(phi_s0 = 0; phi_s0 < sp4C; phi_s0++){
                    local->unk4++;
                    temp_s1 = getPicturePiecePosition(this);
                    addOrRemovePieceFromDisplay(this, temp_s1, 1);
                    local->unk0 |= (1 << temp_s1);
                    item_adjustByDiffWithoutHud(ITEM_26_JIGGY_TOTAL, -1);
                }
                fileProgressFlag_setN(_puzzleFlag(this->actorTypeSpecificField - 1), local->unk4, _puzzleSize(this->actorTypeSpecificField - 1));
                unlockAdditionalActions(this);
            }
            break;

        case 7: //L8038F724
        comusic_playTrack(COMUSIC_65_WORLD_OPENING_B);
        if (this->actorTypeSpecificField == 1) {
            func_80324DBC(1.0f, 0xF7E, 4, NULL, this->marker, gruntyLaughCallback, NULL);
        } else if (this->actorTypeSpecificField == 0xA) {
            func_80324DBC(1.0f, 0xFAC, 4, NULL, this->marker, gruntyLaughCallback, NULL);
        }
        timedFunc_set_1(2.0f, (GenFunction_1) afterPictureComplete, (uintptr_t)this->marker);
        this->lifetime_value = 3.0f;
            break;
    }
    subaddie_set_state(this, next_state);
}


void setInitialJigsawPictureOpacity(Actor *this) {
    Struct70s *temp_v0;
    s32 phi_s0;

    for(phi_s0 = 0; phi_s0 < getPictureCost(this); phi_s0++){
        temp_v0 = func_8034C528(jiggyPositionToID(this, phi_s0));
        if (temp_v0 != 0) {
            setStruct6DsOpacity((Struct6Ds *)temp_v0, isPicturePiecePlaced(this, phi_s0) ? 0xff : 0); // [port] Struct70s* layout-compatible with Struct6Ds* here
        }
    }
}

void addPieces(Actor *this, s32 arg1) {
    if (item_getCount(ITEM_26_JIGGY_TOTAL) > 0) {
        jigsawPicture_setState(this, arg1);
        return;
    }
    comusic_playTrack(COMUSIC_2C_BUZZER);
    if (fileProgressFlag_get(FILEPROG_DE_USED_ALL_YOUR_PUZZLE_PIECES) != 0) {
        jigsawPicture_setState(this, 1);
        return;
    }
    gcdialog_showDialog(0xFBC, 4, NULL, NULL, NULL, NULL);
    fileProgressFlag_set(FILEPROG_DE_USED_ALL_YOUR_PUZZLE_PIECES, 1);
}

// [port] unk38_31 marks a podium that is spawned but not yet revealed by its switch (see boggy3).
Actor *jigsawPicture_draw(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx) {
    Actor *this = marker_getActor(marker);

    if (this->unk38_31) return this;

    return actor_draw(marker, gfx, mtx, vtx);
}

static void revealCcwPuzzlePodium(Actor *this) {
    this->unk38_31 = false;
    this->marker->propPtr->unk8_3 = true;
    if (!fileProgressFlag_get(FILEPROG_54_CCW_PUZZLE_PODIUM_ACTIVE)) {
        __bundle_spawnFromFirstActor(BUNDLE_20__UNKNOWN, this);
        func_80324CFC(0.0f, COMUSIC_43_ENTER_LEVEL_GLITTER, 0x7FFF);
        func_80324D2C(2.1f, COMUSIC_43_ENTER_LEVEL_GLITTER);
        func_8030E6D4(SFX_113_PAD_APPEARS);
    }
}

static s32 jigsawPicture_placedFromFlags(Actor *this) {
    return fileProgressFlag_getN(_puzzleFlag(this->actorTypeSpecificField - 1), _puzzleSize(this->actorTypeSpecificField - 1));
}

static void jigsawPicture_resyncFromFlags(Actor *this) {
    ActorLocal_lair_86F0 *local = (ActorLocal_lair_86F0*)&this->local;
    s32 placed = jigsawPicture_placedFromFlags(this);
    s32 i;

    local->unk0 = 0;
    local->unk4 = 0;
    for(i = 0; i < placed; i++){
        local->unk4++;
        local->unk0 |= (1 << getPicturePiecePosition(this));
    }
    setInitialJigsawPictureOpacity(this);
}

void updateJigsawPictureActor(Actor *this) {
    ActorLocal_lair_86F0 *local;
    s32 sp7C[6]; //buttons
    s32 phi_v1;
    s32 phi_a0;
    s32 sp6C[3]; //joystick
    f32 sp68;
    s32 sp64;
  

    local = (ActorLocal_lair_86F0*)&this->local;
    sp68 = time_getDelta();
    if (!this->initialized) {
        this->initialized = true;
    }

    if (!this->volatile_initialized) {
        // temp_v0 = &D_803947F8[this->actorTypeSpecificField - 1];
        sp64 = fileProgressFlag_getN(_puzzleFlag(this->actorTypeSpecificField - 1), _puzzleSize(this->actorTypeSpecificField - 1));
        local->unk0 = 0;
        local->unk4 = 0;
        local->unk8 = (isBanjoOnPodium(this->marker)) ? 0xff : 1;
        this->has_met_before = true;
        for(phi_v1 = 0; phi_v1 < sp64; phi_v1 ++){
            local->unk4++;
            local->unk0 |= (1 << getPicturePiecePosition(this));
        }
        setInitialJigsawPictureOpacity(this);
        marker_setCollisionScripts(this->marker, onJigsawPodiumCollide, NULL, NULL);
        this->marker->propPtr->unk8_3 = true;
        this->volatile_initialized = true;
        if (this->actorTypeSpecificField == 9) {
            this->unk1C[0] = 8.0f;
            if (!fileProgressFlag_get(FILEPROG_53_CCW_PUZZLE_PODIUM_SWITCH_PRESSED)) {
                // [port] Anchor suppresses the despawn: a despawned podium only comes back when the
                // cube re-streams, which would strand a teammate standing here when the flag arrives.
                if (EventSystem_Should(VB_CCW_PODIUM_DESPAWN, true, this)) {
                    marker_despawn(this->marker);
                } else {
                    this->unk38_31 = true;
                    this->marker->propPtr->unk8_3 = false;
                }
                return;
            }
            revealCcwPuzzlePodium(this);
        }
    }

    // [port] Waiting on the switch: reveal in place the moment the flag lands, ours or a teammate's.
    if (this->unk38_31) {
        if (!fileProgressFlag_get(FILEPROG_53_CCW_PUZZLE_PODIUM_SWITCH_PRESSED)) {
            return;
        }
        revealCcwPuzzlePodium(this);
    }

    if ((this->actorTypeSpecificField == 9) && !fileProgressFlag_get(FILEPROG_54_CCW_PUZZLE_PODIUM_ACTIVE)) {
        this->yaw += this->unk1C[0];
        while(this->yaw >= 360.0f){
            this->yaw -= 360.0f;
        }
        this->unk1C[0] -= 0.089888;
        if (this->unk1C[0] < 0.0f) {
            this->unk1C[0] = 0.0f;
        }
        if (this->marker->unk14_21) {
            s32* sp58 = D_80394824;
            ParticleEmitter *sp54;
            sp54 = partEmitMgr_newEmitter(6);
            particleEmitter_setSprite(sp54, ASSET_710_SPRITE_SPARKLE_PURPLE);
            particleEmitter_setAlpha(sp54, 0xFF);
            particleEmitter_setScaleAndLifetimeRanges(sp54, &D_80394830);
            particleEmitter_setPosition(sp54, this->position);
            sp58[2] = randf() * 255.0f;
            particleEmitter_setRGB(sp54, sp58);
            particleEmitter_setSpawnPositionRange(sp54, -30.0f, -40.0f, -30.0f, 30.0f, 20.0f, 30.0f);
            particleEmitter_emitN(sp54, 6);
        }
    }
    controller_copyFaceButtons(0, sp7C);
    controller_copySideButtons(0, sp6C);
    func_8038EDBC(this);
    if (local->unk4 != jigsawPicture_placedFromFlags(this) && EventSystem_Should(VB_JIGSAW_PICTURE_RESYNC, false, this)) {
        jigsawPicture_resyncFromFlags(this);
    }
    if (this->state != JIGSAW_PICTURE_LEAVE_PODIUM && !port_jigsawPedestal_isSelf(this->actorTypeSpecificField)) {
        jigsawPicture_setState(this, JIGSAW_PICTURE_LEAVE_PODIUM);
        return;
    }

    switch (this->state) {
        case JIGSAW_PICTURE_LEAVE_PODIUM:
            if (!this->has_met_before && (!player_isStableWithExtraSteps() || !func_8028FB48(0x08000000))) {
                this->has_met_before = TRUE;
            }
            if (subaddie_playerIsWithinSphereAndActive(this, 300)) {
                if ((this->actorTypeSpecificField == 0xA) && !fileProgressFlag_get(FILEPROG_F6_SEEN_DOOR_OF_GRUNTY_PUZZLE_PODIUM)) {
                    phi_a0 = (item_getCount(ITEM_26_JIGGY_TOTAL) < D_803947F8[this->actorTypeSpecificField - 1].cost) ? 0xFAB : 0xFC0;
                    if (gcdialog_showDialog(phi_a0, 0, NULL, NULL, NULL, NULL)) {
                        fileProgressFlag_set(FILEPROG_F6_SEEN_DOOR_OF_GRUNTY_PUZZLE_PODIUM, true);
                    }
                } else if (this->actorTypeSpecificField == 1) {
                    progressDialog_showDialogMaskZero(FILEPROG_A7_NEAR_PUZZLE_PODIUM_TEXT);
                }
            }
            if (isBanjoOnPodium(this->marker) && this->has_met_before && !isPictureComplete(this) && (player_movementGroup() == BSGROUP_0_NONE || player_movementGroup() == BSGROUP_8_TROT)) {
                if (port_jigsawPedestal_tryClaim(this->actorTypeSpecificField)) {
                    jigsawPicture_setState(this, 2);
                } else if (sBuzzedPedestalField != this->actorTypeSpecificField) {
                    comusic_playTrack(COMUSIC_2C_BUZZER);
                    sBuzzedPedestalField = this->actorTypeSpecificField;
                }
            } else if (sBuzzedPedestalField == this->actorTypeSpecificField) {
                sBuzzedPedestalField = 0;
            }
            break;

        case 4: //L8038FE28
            { CALL_EVENT(OnJigsawPodiumInput, this->actorTypeSpecificField); }
            if ((gcdialog_getCurrentTextId() != 0xF7C) && (gcdialog_getCurrentTextId() != 0xF7D)) {
                if (sp7C[FACE_BUTTON(BUTTON_A)] == 1) {
                    addPieces(this, 5);
                } else if (sp7C[FACE_BUTTON(BUTTON_B)] == 1) {
                    jigsawPicture_setState(this, 1);
                } else if ((sp6C[SIDE_BUTTON(BUTTON_Z)] == 1) && fileProgressFlag_get(FILEPROG_E0_CAN_PLACE_ALL_PUZZLE_PIECES)) {
                    addPieces(this, 6);
                } else if (sp7C[FACE_BUTTON(BUTTON_C_DOWN)] == 1) {
                    if (local->unk4) {
                        jigsawPicture_setState(this, 8);
                    } else {
                        comusic_playTrack(COMUSIC_2C_BUZZER);
                        jigsawPicture_setState(this, 1);
                    }
                }
            }
            break;

        case 5: //L8038FF00
        case 6: //L8038FF00
        case 8: //L8038FF00
            if (this->lifetime_value > 0.0f) {
                this->lifetime_value -= sp68;
            } else {
                jigsawPicture_setState(this, isPictureComplete(this) ? 7 :4);
            }
            break;

        case 7: //L8038FF50
            if (this->lifetime_value > 0.0f) {
                this->lifetime_value -= sp68;
            } else {
                jigsawPicture_setState(this, 1);
            }
            break;
    }

    {
        s32 pad;
        f32 sp44[3];
        s32 pad2;
        this->marker->isBanjoOnTop = false;
        player_getPosition(sp44);
        if (ml_distanceSquared_vec3f(sp44, this->position) < 250000.0f) {
            if (!this->unk38_0) {
                itemPrint_reset();
                this->unk38_0 = true;
            }
            code_73640_printItemCount(0x2B);
        }
        else if (this->unk38_0) {
            func_802FAD64(0x2B);
            this->unk38_0 = false;
        }
    }
}
