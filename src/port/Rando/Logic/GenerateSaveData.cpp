#include "Logic.h"
#include "port/UI/Notification.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include "port/Save/Types.h"

extern "C" {
void ability_setHasUsed(enum ability_e move);

void item_setMaxCount(s32 item);
}

// clang-format off
std::map<ability_e, std::pair<const char*, const char*>> abilityLoadoutMap = {
    { ABILITY_0_BARGE,              { "Beak Barge",         CVAR_RANDOMIZER_SETTING("Loadout.Abilities.BeakBarge") } },
    { ABILITY_1_BEAK_BOMB,          { "Beak Bomb",          CVAR_RANDOMIZER_SETTING("Loadout.Abilities.BeakBomb") } },
    { ABILITY_2_BEAK_BUSTER,        { "Beak Buster",        CVAR_RANDOMIZER_SETTING("Loadout.Abilities.BeakBuster") } },
    { ABILITY_3_CAMERA_CONTROL,     { "Camera Control",     CVAR_RANDOMIZER_SETTING("Loadout.Abilities.CameraControl") } },
    { ABILITY_4_CLAW_SWIPE,         { "Claw Swipe",         CVAR_RANDOMIZER_SETTING("Loadout.Abilities.ClawSwipe") } },
    { ABILITY_5_CLIMB,              { "Climb",              CVAR_RANDOMIZER_SETTING("Loadout.Abilities.Climb") } },
    { ABILITY_6_EGGS,               { "Eggs",               CVAR_RANDOMIZER_SETTING("Loadout.Abilities.Eggs") } },
    { ABILITY_7_FEATHERY_FLAP,      { "Feathery Flap",      CVAR_RANDOMIZER_SETTING("Loadout.Abilities.FeatheryFlap") } },
    { ABILITY_8_FLAP_FLIP,          { "Flap Flip",          CVAR_RANDOMIZER_SETTING("Loadout.Abilities.FlapFlip") } },
    { ABILITY_9_FLIGHT,             { "Flight",             CVAR_RANDOMIZER_SETTING("Loadout.Abilities.Flight") } },
    { ABILITY_A_HOLD_A_JUMP_HIGHER, { "Jump Higher",        CVAR_RANDOMIZER_SETTING("Loadout.Abilities.JumpHigher") } },
    { ABILITY_B_RATATAT_RAP,        { "Rat-a-tat Rap",      CVAR_RANDOMIZER_SETTING("Loadout.Abilities.RatatatRap") } },
    { ABILITY_C_ROLL,               { "Roll",               CVAR_RANDOMIZER_SETTING("Loadout.Abilities.Roll") } },
    { ABILITY_D_SHOCK_JUMP,         { "Shock Jump",         CVAR_RANDOMIZER_SETTING("Loadout.Abilities.ShockJump") } },
    { ABILITY_E_WADING_BOOTS,       { "Wading Boots",       CVAR_RANDOMIZER_SETTING("Loadout.Abilities.WadingBoots") } },
    { ABILITY_F_DIVE,               { "Dive",               CVAR_RANDOMIZER_SETTING("Loadout.Abilities.Dive") } },
    { ABILITY_10_TALON_TROT,        { "Talon Trot",         CVAR_RANDOMIZER_SETTING("Loadout.Abilities.TalonTrot") } },
    { ABILITY_11_TURBO_TALON,       { "Turbo Talon",        CVAR_RANDOMIZER_SETTING("Loadout.Abilities.TurboTalon") } },
    { ABILITY_12_WONDERWING,        { "Wonderwing",         CVAR_RANDOMIZER_SETTING("Loadout.Abilities.Wonderwing") } },
    { ABILITY_13_1ST_NOTEDOOR,      { "First Note Door",    CVAR_RANDOMIZER_SETTING("Loadout.Abilities.NoteDoor") } },
};

std::map<item_e, std::pair<const char*, const char*>> itemLoadoutMap = {
    { ITEM_D_EGGS,          { "Blue Eggs",      CVAR_RANDOMIZER_SETTING("Loadout.Items.BlueEggs") } },
    { ITEM_F_RED_FEATHER,   { "Red Feathers",   CVAR_RANDOMIZER_SETTING("Loadout.Items.RedFeathers") } },
    { ITEM_10_GOLD_FEATHER, { "Gold Feathers",  CVAR_RANDOMIZER_SETTING("Loadout.Items.GoldFeathers") } },
};

std::vector<file_progress_e> progressLoadout = {
    FILEPROG_3_MUSIC_NOTE_TEXT,
    FILEPROG_4_MUMBO_TOKEN_TEXT,
    FILEPROG_5_BLUE_EGG_TEXT,
    FILEPROG_6_RED_FEATHER_TEXT,
    FILEPROG_7_GOLD_FEATHER_TEXT,
    FILEPROG_8_ORANGE_TEXT,
    FILEPROG_9_GOLD_BULLION_TEXT,
    FILEPROG_A_HONEYCOMB_TEXT,
    FILEPROG_B_EMPTY_HONEYCOMB_TEXT,
    FILEPROG_C_EXTRA_LIFE_TEXT,
    FILEPROG_D_BEEHIVE_TEXT,
    FILEPROG_E_JINJO_TEXT,
    FILEPROG_F_HAS_TOUCHED_PIRANHA_WATER,
    FILEPROG_10_HAS_TOUCHED_SAND_EEL_SAND,
    FILEPROG_11_HAS_MET_MUMBO, 
    FILEPROG_12_HAS_TRANSFORMED_BEFORE,
    FILEPROG_14_HAS_TOUCHED_FP_ICY_WATER,
    FILEPROG_16_STOOD_ON_JIGSAW_PODIUM,
    FILEPROG_17_HAS_HAD_ENOUGH_JIGSAW_PIECES,
    FILEPROG_1B_MET_YELLOW_FLIBBITS,
    FILEPROG_1D_MMM_DINNING_ROOM_CUTSCENE,
    FILEPROG_55_FF_BK_SQUARE_INSTRUCTIONS,
    FILEPROG_56_FF_PICTURE_SQUARE_INSTRUCTIONS,
    FILEPROG_57_FF_MUSIC_SQUARE_INSTRUCTIONS,
    FILEPROG_58_FF_MINIGAME_SQUARE_INSTRUCTIONS,
    FILEPROG_59_FF_GRUNTY_SQUARE_INSTRUCTIONS,
    FILEPROG_5A_FF_DEATH_SQUARE_INSTRUCTIONS,
    FILEPROG_5B_FF_JOKER_SQUARE_INSTRUCTIONS,
    FILEPROG_82_MET_TWINKLIES,
    FILEPROG_83_MAGIC_GET_WEAK_TEXT,
    FILEPROG_84_MAGIC_ALL_GONE_TEXT,
    FILEPROG_86_HAS_TOUCHED_MMM_THORN_HEDGE,
    FILEPROG_88_TRIED_LOGGO_AS_BEAR,
    FILEPROG_8F_MET_BEE_INFESTED_BEEHIVE,
    FILEPROG_96_MET_BRENTILDA,
    FILEPROG_97_ENTERED_LAIR_TEXT,
    FILEPROG_98_EXITED_LEVEL_TEXT,
    FILEPROG_99_PAST_50_NOTE_DOOR_TEXT,
    FILEPROG_A7_NEAR_PUZZLE_PODIUM_TEXT,
    FILEPROG_A8_HAS_DIED,
    FILEPROG_AB_SWIM_OILY_WATER,
    FILEPROG_AC_DIVE_OILY_WATER,
    FILEPROG_BA_HAS_SEEN_TREX_TEXT,
    FILEPROG_BD_ENTER_LAIR_CUTSCENE,
    FILEPROG_C1_BADDIES_ESCAPE_TEXT,
    FILEPROG_DB_SKIPPED_TUTORIAL,
    FILEPROG_DC_HAS_HAD_ENOUGH_TOKENS_BEFORE,
    FILEPROG_DD_HAS_TOUCHED_CCW_ICY_WATER,
    FILEPROG_DE_USED_ALL_YOUR_PUZZLE_PIECES,
    FILEPROG_DF_CAN_REMOVE_ALL_PUZZLE_PIECES,
    FILEPROG_E0_CAN_PLACE_ALL_PUZZLE_PIECES,
    FILEPROG_F3_MET_DINGPOT,
    // Save for later, not sure if game breaks if set at start
    //FILEPROG_F4_ENTER_FF_CUTSCENE,

};

std::vector<RandoCheckId> smRandoCheckIdList = {
    RC_SM_EMPTY_HONEYCOMB_COLLIWOBBLE,
    RC_SM_EMPTY_HONEYCOMB_QUARRIES,
    RC_SM_EMPTY_HONEYCOMB_STUMP,
    RC_SM_EMPTY_HONEYCOMB_TREE,
    RC_SM_EMPTY_HONEYCOMB_UNDERWATER,
    RC_SM_EMPTY_HONEYCOMB_WATERFALL,
    RC_SM_MOLEHILL_ATTACK,
    RC_SM_MOLEHILL_BEAK_BARGE,
    RC_SM_MOLEHILL_CAMERA_CONTROL,
    RC_SM_MOLEHILL_CLIMB,
    RC_SM_MOLEHILL_DIVE,
    RC_SM_MOLEHILL_JUMP,
};
// clang-format on

void Rando::Logic::InitializeSaveData(SaveData* saveData) {
    // RandoSaveCheck - Initialize
    for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
        RandoSaveCheck randoSaveCheck = {
            .name = randoStaticCheck.name,
            .randoItemId = randoStaticCheck.randoItemId,
            // .randoItemId = Rando::StaticData::GetRandoItemByActorId((actor_e)randoStaticCheck.actorId),
            .randoCollectionId = randoStaticCheck.collectionId,
            .isShuffled = false,
            .eligible = false,
            .skipped = false,
        };

        saveData->shipSaveData.randoSaveData.randoSaveCheck[randoCheckId] = randoSaveCheck;
    }

    for (auto& [randoOptionId, randoStaticOption] : Rando::StaticData::Options) {
        RandoSaveOption randoSaveOption = {
            .name = randoStaticOption.name,
            .optionValue = CVarGetInteger(randoStaticOption.cvar, randoStaticOption.defaultValue),
        };

        saveData->shipSaveData.randoSaveData.randoSaveOption[randoOptionId] = randoSaveOption;
    }

    for (int i = RANDO_INF_UNKNOWN; i < RANDO_INF_MAX; i++) {
        saveData->shipSaveData.randoSaveData.randoSaveFlag[i].flagState = 0;
    }
}

void Rando::Logic::GenerateSaveData(SaveData* saveData) {
    for (auto& object : RANDO_SAVE_CHECKS) {
        saveData->shipSaveData.randoSaveData.randoSaveCheck[object.randoCheckId] = object;
    }

    for (int f = RANDO_INF_UNKNOWN; f < RANDO_INF_MAX; f++) {
        saveData->shipSaveData.randoSaveData.randoSaveFlag[f].flagState = RANDO_SAVE_FLAGS[f].flagState;
    }
}

void Rando::Logic::GrantStartingLoadout() {
    for (auto& [ability, abilityInfo] : abilityLoadoutMap) {
        if (ability == ABILITY_A_HOLD_A_JUMP_HIGHER || ability == ABILITY_13_1ST_NOTEDOOR ||
            CVarGetInteger(abilityInfo.second, 0)) {
            ability_setLearned(ability, true);
            ability_setHasUsed(ability);
        }
    }
    for (auto& [item, itemInfo] : itemLoadoutMap) {
        if (CVarGetInteger(itemInfo.second, 0)) {
            item_setMaxCount(item);
        }
    }
}

void Rando::Logic::GrantFileProgressFlags() {
    for (auto& fileprog : progressLoadout) {
        fileProgressFlag_set(fileprog, 1);
    }
}

void Rando::Logic::GrantSpiralMountainChecks() {
    if (!CVarGetInteger(CVAR_ENHANCEMENT("Gameplay.SkipSMTutorial"), 0)) {
        return;
    }

    for (auto& smCheckId : smRandoCheckIdList) {
        RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[smCheckId];

        if (!randoSaveCheck.isShuffled) {
            continue;
        }

        CustomObject::CheckObtainedEX(smCheckId, true);
        if (Rando::StaticData::Items[randoSaveCheck.randoItemId].randoItemType == RITYPE_MOLEHILL) {
            ability_setLearned((ability_e)randoSaveCheck.randoCollectionId, true);
            ability_setHasUsed((ability_e)randoSaveCheck.randoCollectionId);
        }
    }
}