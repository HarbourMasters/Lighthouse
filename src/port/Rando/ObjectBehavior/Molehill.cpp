#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#define MOLETABLE_SIZE 16
#define BRIDGE_REQUIREMENT 6

typedef struct {
    s16 teach_text_id;
    s16 refresher_text_id;
    s8 camera_node;
    s8 ability;
} ChMoleDescription;

extern "C" {
void func_80347A14(s32 arg0);
s32 item_adjustByDiffWithHud(enum item_e item, s32 diff);
void ability_unlock(enum ability_e);
s32 mapSpecificFlags_get(s32 i);
void mapSpecificFlags_set(s32 i, s32 val);
}

// clang-format off
ChMoleDescription moleDialogTable[MOLETABLE_SIZE] = {
    //{ ASSET_DF3_DIALOG_BOTTLES_INTRODUCTION,            ASSET_E08_DIALOG_BOTTLES_FIND_ANOTHER_MOLEHILL,     0x1, -1 },
    { ASSET_DF4_DIALOG_BOTTLES_CAMERA_CONTROL_LEARN,    ASSET_DF5_DIALOG_BOTTLES_CAMERA_CONTROL_REFRESHER,  0x3, ABILITY_3_CAMERA_CONTROL },
    { ASSET_DFB_DIALOG_BOTTLES_DIVE_LEARN,              ASSET_DFE_DIALOG_BOTTLES_DIVE_REFRESHER,            0x5, ABILITY_F_DIVE },
    { -1,                                               ASSET_E00_DIALOG_BOTTLES_ATTACK_REFRESHER,          0x6, ABILITY_4_CLAW_SWIPE },
    { ASSET_E04_DIALOG_BOTTLES_BEAK_BARGE_LEARN,        ASSET_E06_DIALOG_BOTTLES_BEAK_BARGE_REFRESHER,      0x8, ABILITY_0_BARGE },
    { -1,                                               ASSET_DFA_DIALOG_BOTTLES_JUMP_REFRESHER,            0x4, ABILITY_8_FLAP_FLIP },
    { ASSET_E01_DIALOG_BOTTLES_CLIMB_LEARN,             ASSET_E03_DIALOG_BOTTLES_CLIMB_REFRESHER,           0x7, ABILITY_5_CLIMB },
    //{ ASSET_E10_DIALOG_BOTTLES_BRIDGE_BROKEN,           ASSET_E11_DIALOG_BOTTLES_BRIDGE_STILL_BROKEN,       0x11, -1 },
    { ASSET_C23_DIALOG_BEAKBOMB_LEARN,                  ASSET_C24_DIALOG_BEAKBOMB_REFRESHER,                0x0F, ABILITY_1_BEAK_BOMB },
    { ASSET_B47_DIALOG_EGGS_LEARN,                      ASSET_B4B_DIALOG_EGGS_REFRESHER,                    0x16, ABILITY_6_EGGS },
    { ASSET_B48_DIALOG_BEAKBUSTER_LEARN,                ASSET_B4C_DIALOG_BEAKBUSTER_REFRESHER,              0x17, ABILITY_2_BEAK_BUSTER },
    { ASSET_B49_DIALOG_TALON_TROT_LEARN,                ASSET_B4A_DIALOG_TALON_TROT_REFRESHER,              0x18, ABILITY_10_TALON_TROT },
    { ASSET_A1F_DIALOG_SHOCKJUMP_LEARN,                 ASSET_A23_DIALOG_SHOCKJUMP_REFRESHER,               0x0C, ABILITY_D_SHOCK_JUMP },
    { ASSET_A20_DIALOG_FLY_LEARN,                       ASSET_A22_DIALOG_FLY_REFRESHER,                     0x0D, ABILITY_9_FLIGHT },
    { ASSET_D35_DIALOG_WONDERWING_LEARN,                ASSET_D36_DIALOG_WONDERWING_REFRESHER,              0x01, ABILITY_12_WONDERWING },
    { ASSET_C88_DIALOG_WADING_BOOTS_LEARN,              ASSET_C89_DIALOG_WADING_BOOTS_REFRESHER,            0x10, ABILITY_E_WADING_BOOTS },
    { ASSET_A84_DIALOG_TURBOTRAINERS_LEARN,             ASSET_A85_DIALOG_TURBOTRAINERS_REFRESHER,           0x19, ABILITY_11_TURBO_TALON },
    { ASSET_F64_DIALOG_NOTEDOORS_LEARN,                 ASSET_F65_DIALOG_NOTEDOORS_REFRESHER,               0x0E, ABILITY_13_1ST_NOTEDOOR },
};

std::vector<RandoCheckId> spiralMountainBridge = {
    RC_SM_MOLEHILL_JUMP,
    RC_SM_MOLEHILL_CAMERA_CONTROL,
    RC_SM_MOLEHILL_ATTACK,
    RC_SM_MOLEHILL_DIVE,
    RC_SM_MOLEHILL_CLIMB,
    RC_SM_MOLEHILL_BEAK_BARGE,
};
// clang-format on

ChMoleDescription result;

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MOLEHILLS].optionValue

ChMoleDescription GetMoleDescriptionByAbility(int16_t abilityId) {
    result.ability = -1;
    for (int i = 0; i < MOLETABLE_SIZE; i++) {
        if (moleDialogTable[i].ability == abilityId) {
            result = moleDialogTable[i];
            break;
        }
    }

    return result;
}

bool CheckBridgeState() {
    int32_t smBridgeCheck = 0;
    if (!mapSpecificFlags_get(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED)) {
        for (auto& check : spiralMountainBridge) {
            if (Rando::Logic::GetShuffledObject(check).obtained) {
                smBridgeCheck++;
            }
        }
    }

    return smBridgeCheck == BRIDGE_REQUIREMENT ? true : false;
}

void SetSpiralMountainFlags() {
    mapSpecificFlags_set(SM_SPECIFIC_FLAG_1_TALKED_TO_BOTTLES, true);
}

static void MarkBridgeRepairedDialogComplete() {
    CALL_EVENT(SetRandoInfFlag, RANDO_INF_BRIDGE_REPAIRED_DIALOG_COMPLETE, true);
}

void Rando::ObjectBehavior::InitMolehillBehavior() {
    COND_VB_SHOULD(VB_OVERRIDE_MOLEHILL_ABILITY, EVENT_PRIORITY_NORMAL, true, {
        Actor* molehillActor = va_arg(args, Actor*);
        s32* textId = va_arg(args, s32*);
        s32* isLearned = va_arg(args, s32*);

        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        if (molehillActor->actorTypeSpecificField == 8) {
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(
            molehillActor->position_x, molehillActor->position_y, molehillActor->position_z);

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        RandoSaveCheck shuffledMolehill = Rando::Logic::GetShuffledObject(randoCheckId);
        ChMoleDescription moleInfo = GetMoleDescriptionByAbility(shuffledMolehill.randoCollectionId);

        if (moleInfo.ability >= ABILITY_0_BARGE) {
            *should = true;

            func_80347A14(0);
            molehillActor->has_met_before = true;

            switch (moleInfo.ability) {
                case ABILITY_4_CLAW_SWIPE:
                    *textId = (s32)moleInfo.refresher_text_id;
                    ability_unlock(ABILITY_4_CLAW_SWIPE);
                    ability_unlock(ABILITY_C_ROLL);
                    ability_unlock(ABILITY_B_RATATAT_RAP);
                    break;
                case ABILITY_6_EGGS:
                    *textId = (s32)moleInfo.refresher_text_id;
                    ability_unlock((ability_e)moleInfo.ability);
                    item_adjustByDiffWithHud(ITEM_D_EGGS, 50);
                    break;
                case ABILITY_8_FLAP_FLIP:
                    *textId = (s32)moleInfo.refresher_text_id;
                    ability_unlock(ABILITY_A_HOLD_A_JUMP_HIGHER);
                    ability_unlock(ABILITY_7_FEATHERY_FLAP);
                    ability_unlock(ABILITY_8_FLAP_FLIP);
                    break;
                case ABILITY_9_FLIGHT:
                    *textId = (s32)moleInfo.refresher_text_id;
                    ability_unlock((ability_e)moleInfo.ability);
                    item_adjustByDiffWithHud(ITEM_F_RED_FEATHER, 25);
                    break;
                case ABILITY_12_WONDERWING:
                    *textId = (s32)moleInfo.refresher_text_id;
                    ability_unlock((ability_e)moleInfo.ability);
                    item_adjustByDiffWithHud(ITEM_10_GOLD_FEATHER, 5);
                    break;
                default:
                    *textId = (s32)moleInfo.refresher_text_id;
                    ability_unlock((ability_e)moleInfo.ability);
                    break;
            }

            CustomObject::CheckObtainedEX(shuffledMolehill.randoCheckId);

            if (map_getLevel(gsworld_getMap()) == LEVEL_B_SPIRAL_MOUNTAIN) {
                if (CheckBridgeState()) {
                    SetSpiralMountainFlags();
                }
            }
        }
    })

    COND_VB_SHOULD(VB_OVERRIDE_BOTTLES_TEXT_CALLBACK, EVENT_PRIORITY_NORMAL, true, {
        Actor* molehillActor = va_arg(args, Actor*);

        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        if (CheckBridgeState() && !RANDO_SAVE_FLAGS[RANDO_INF_BRIDGE_REPAIRED_DIALOG_COMPLETE].flagState) {
            *should = true;
        }
    })

    COND_HOOK(OnCheckSpiralMountainAbilities, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnCheckSpiralMountainAbilities* ev = (OnCheckSpiralMountainAbilities*)event;
        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        if (mapSpecificFlags_get(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED)) {
            mapSpecificFlags_set(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED, false);
        }

        if (CheckBridgeState()) {
            mapSpecificFlags_set(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED, true);
            if (!RANDO_SAVE_FLAGS[RANDO_INF_BRIDGE_REPAIRED_DIALOG_COMPLETE].flagState) {
                MarkBridgeRepairedDialogComplete();
            }
            event->Cancelled = true;
            ev->result = true;
        }
    })
}
