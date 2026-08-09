// BanjoDecomp: core2/bs/bEggAss.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "core2/ba/physics.h"
#include "core2/yaw.h"

/* .bss */
u8 D_8037D2E0;
u8 D_8037D2E1;

/* .code */
void bseggass_init(void){
    baanim_playForDuration_once(ASSET_2B_ANIM_BSEGGASS, 1.0f);
    code_14420_setUpdateTypes(1, YAW_STATE_3_BOUNDED, 1, BA_PHYSICS_LOCKED_ROTATION);
    yaw_setVelocityBounded(350.0f, 14.0f);
    baphysics_set_target_horizontal_velocity(0.0f);
    modelAppendages_setKazooiesAssVisibility(true);
    D_8037D2E0 = (D_8037D2E1 = 1);
    bainput_enable(5,0);
}

void bseggass_update(void) {
    s32 next_state;
    AnimCtrl *plyr_mvmt;
    s32 has_eggs;
    s32 sp28;
    s32 fill1;
    s32 fill2;

    next_state = 0;
    plyr_mvmt = baanim_getAnimCtrlPtr();
    has_eggs = (item_empty(ITEM_D_EGGS) == 0);
    if (bainput_should_poop_egg()) {
        if (has_eggs)
            D_8037D2E0 = ml_min_w(D_8037D2E0 + 1, 3);
        else 
            func_80346C10((enum bs_e *)&sp28, -1, 0, ITEM_D_EGGS, 0);
    }
    if (has_eggs) {
        if (anctrl_isAt(plyr_mvmt, 0.3837f)) {
            // [port] Romhack gate
            s32 rate = 28000;
            EventSystem_Should(VB_EGG_FIRE_SFX, true, 2, &rate);
            func_8030E760(SFX_3E_POOP_NOISE, 1.4f, rate);
            commonParticle_new(COMMON_PARTICLE_4_EGG_ASS, 1);
            item_dec(ITEM_D_EGGS);
            ability_use(7);
        }
        if ((anctrl_isAt(plyr_mvmt,  0.4885f)) &&  (D_8037D2E1 < D_8037D2E0)) {
            anctrl_setStart(plyr_mvmt, 0.349f);
            anctrl_start(plyr_mvmt, "bsbeggass.c", 0x5E);
            D_8037D2E1++;
        }
    }
    if (anctrl_isStopped(plyr_mvmt)) {
        next_state = (bakey_held(BUTTON_Z))? BS_7_CROUCH : BS_1_IDLE;
    } else if (0.6 < (f64) anctrl_getAnimTimer(plyr_mvmt)) {
        next_state = func_802ADCD4(0);
    }
    if (player_shouldFall())
        next_state = BS_2F_FALL;
    bs_setState(next_state);
}


void bseggass_end(void){
    bainput_enable(5, 1);
    baphysics_reset_gravity();
    modelAppendages_setKazooiesAssVisibility(false);
}
