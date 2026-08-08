#include "ItemQueue.h"

#include <queue>
#include <unordered_map>
#include <map>

#include "port/UI/Notification.h"
#include "port/UI/UIWidgets.hpp"
#include <libultraship/bridge.h>
#include "port/ShipInit.hpp"
#include "port/Rando/Logic/Logic.h"
#include "actor.h"
#include "port/Enhancements/Retention/Retention.h"

extern "C" {
#include "functions.h"
extern u8 sHoneycombScore[3];
extern struct {
    u8 D_803832C0[0xD];
    u8 D_803832CD[0xD];
} jiggyscore;
#define MUMBO_TOKEN_COUNT 126
#define MUMBOSCORE_SIZE (((MUMBO_TOKEN_COUNT - 1 + 7) & ~7) / 8)
extern u8 sMumboTokenScore[MUMBOSCORE_SIZE];
extern u8 D_80385FF0[0xE];

extern ActorInfo chJinjoBlue;
extern ActorInfo chJinjoGreen;
extern ActorInfo chJinjoYellow;
extern ActorInfo chJinjoPink;
extern ActorInfo chJinjoOrange;
extern void spawnOrbit();
}

constexpr u8 kAllJinjos = 0x1F; // all five color bits collected

#define CVAR_NAME_SHOW_COLLISION_NOTIFICATIONS "gRandoSettings.RandoNotifications"
#define CVAR_SHOW_COLLISION_NOTIFICATIONS CVarGetInteger(CVAR_NAME_SHOW_COLLISION_NOTIFICATIONS, 0)

#define JIGGY_ID_MULTIPLIER(levelId) (1 + (10 * (levelId - 1)))
#define HONEYCOMB_ID_MULTIPLIER(levelId) (1 + (2 * (levelId - 1)))

static std::queue<RandoCheckId> itemQueue;

std::unordered_map<actor_e, UIWidgets::Colors> itemColors = {
    { ACTOR_1_UNKNOWN, UIWidgets::Colors::Brown },           { ACTOR_52_BLUE_EGG, UIWidgets::Colors::Cyan },
    { ACTOR_47_EMPTY_HONEYCOMB, UIWidgets::Colors::Yellow }, { ACTOR_49_EXTRA_LIFE, UIWidgets::Colors::Yellow },
    { ACTOR_50_HONEYCOMB, UIWidgets::Colors::Yellow },       { ACTOR_46_JIGGY, UIWidgets::Colors::Yellow },
    { ACTOR_60_JINJO_BLUE, UIWidgets::Colors::SkyBlue },     { ACTOR_62_JINJO_GREEN, UIWidgets::Colors::Green },
    { ACTOR_5F_JINJO_ORANGE, UIWidgets::Colors::Orange },    { ACTOR_61_JINJO_PINK, UIWidgets::Colors::Pink },
    { ACTOR_5E_JINJO_YELLOW, UIWidgets::Colors::Yellow },    { ACTOR_12C_MOLEHILL, UIWidgets::Colors::Cyan },
    { ACTOR_2D_MUMBO_TOKEN, UIWidgets::Colors::Gray },       { ACTOR_51_MUSIC_NOTE, UIWidgets::Colors::Yellow },
    { ACTOR_25E_SNS_EGG, UIWidgets::Colors::Pink },          { ACTOR_25D_ICE_KEY, UIWidgets::Colors::White },
};

std::unordered_map<RandoItemId, UIWidgets::Colors> snsColors = {
    { RI_STOP_N_SWOP_EGG_YELLOW, UIWidgets::Colors::Yellow }, { RI_STOP_N_SWOP_EGG_RED, UIWidgets::Colors::Red },
    { RI_STOP_N_SWOP_EGG_GREEN, UIWidgets::Colors::Green },   { RI_STOP_N_SWOP_EGG_BLUE, UIWidgets::Colors::Blue },
    { RI_STOP_N_SWOP_EGG_PINK, UIWidgets::Colors::Pink },     { RI_STOP_N_SWOP_EGG_CYAN, UIWidgets::Colors::Cyan },
    { RI_STOP_N_SWOP_ICE_KEY, UIWidgets::Colors::White },
};

std::unordered_map<int16_t, RandoCheckId> jinjoJiggyChecks = {
    { LEVEL_1_MUMBOS_MOUNTAIN, RC_MM_JIGGY_JINJO },      { LEVEL_2_TREASURE_TROVE_COVE, RC_TTC_JIGGY_JINJO },
    { LEVEL_3_CLANKERS_CAVERN, RC_CC_JIGGY_JINJO },      { LEVEL_4_BUBBLEGLOOP_SWAMP, RC_BGS_JIGGY_JINJO },
    { LEVEL_5_FREEZEEZY_PEAK, RC_FP_JIGGY_JINJO },       { LEVEL_7_GOBIS_VALLEY, RC_GV_JIGGY_JINJO },
    { LEVEL_8_CLICK_CLOCK_WOOD, RC_CCW_JIGGY_JINJO },    { LEVEL_9_RUSTY_BUCKET_BAY, RC_RBB_JIGGY_JINJO },
    { LEVEL_A_MAD_MONSTER_MANSION, RC_MMM_JIGGY_JINJO },
};

void ItemQueue::Process() {
    if (itemQueue.size() < 1) {
        return;
    }

    RandoCheckId randoCheckId = itemQueue.front();

    // Grant item
    RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
    if (!randoSaveCheck.received) {
        ItemQueue::GiveItem(randoSaveCheck.randoItemId);
        ItemQueue::SendNotification(randoSaveCheck.randoItemId);
        Rando::StaticData::ModifyRandoInfFlagState(randoCheckId);
        // RANDO_SAVE_CHECKS[randoCheckId].received = true;
    }

    itemQueue.pop();
}

void ItemQueue::GiveItem(RandoItemId randoItemId) {
    RandoItemType itemType = Rando::StaticData::Items[randoItemId].randoItemType;
    actor_e actorId = (actor_e)Rando::StaticData::Items[randoItemId].actorId;
    int16_t worldId = Rando::StaticData::Items[randoItemId].worldId;
    uint16_t combId;
    uint16_t maxHoneycombs;
    uint32_t jiggyId;
    ActorInfo* actorInfo;
    int32_t playerPosition[3];
    Actor* customActor;
    uint8_t collectedJinjos;

    switch (itemType) {
        case RITYPE_BLUE_EGG:
            coMusicPlayer_playMusic(COMUSIC_C_EGG_COLLECTED, 32000);
            item_inc(ITEM_D_EGGS);
            break;
        case RITYPE_EMPTY_HONEYCOMB:
            combId = HONEYCOMB_ID_MULTIPLIER(worldId);
            maxHoneycombs = worldId == LEVEL_B_SPIRAL_MOUNTAIN ? 6 : 2;
            if (worldId > LEVEL_6_LAIR) {
                combId = HONEYCOMB_ID_MULTIPLIER(worldId - 1);
            }
            for (int i = 0; i < (maxHoneycombs); i++) {
                if ((sHoneycombScore[((combId + i) - 1) / 8] & (1 << ((combId + i) & 7))) == 0) {
                    honeycombscore_set((honeycomb_e)(combId + i), 1);
                    break;
                }
            }
            coMusicPlayer_playMusic(COMUSIC_17_EMPTY_HONEYCOMB_COLLECTED, 28000);
            item_inc(ITEM_13_EMPTY_HONEYCOMB);
            if (!(item_getCount(ITEM_13_EMPTY_HONEYCOMB) < 6)) {
                gcpausemenu_80314AC8(0);
            }
            break;
        case RITYPE_EXTRA_LIFE:
            coMusicPlayer_playMusic(COMUSIC_15_EXTRA_LIFE_COLLECTED, 0x7FFF);
            item_inc(ITEM_16_LIFE);
            break;
        case RITYPE_HONEYCOMB:
            coMusicPlayer_playMusic(COMUSIC_16_HONEYCOMB_COLLECTED, 28000);
            item_inc(ITEM_14_HEALTH);
            break;
        case RITYPE_JIGGY:
            jiggyId = JIGGY_ID_MULTIPLIER(worldId);
            for (int32_t i = jiggyId; i <= (jiggyId + 9); i++) {
                if ((jiggyscore.D_803832C0[(i - 1) / 8] & (1 << (i & 7))) == 0) {
                    jiggyscore_setCollected(i, 1);
                    break;
                }
            }
            coMusicPlayer_playMusic(COMUSIC_D_JINGLE_JIGGY_COLLECTED, -1);
            item_adjustByDiffWithoutHud(ITEM_26_JIGGY_TOTAL, 1);
            item_inc(ITEM_E_JIGGY);
            spawnOrbit();
            break;
        case RITYPE_JINJO:
            if (worldId == map_getLevel(gsworld_getMap())) {
                item_adjustByDiffWithHud(ITEM_12_JINJOS, jinjoBitFromActor(actorId));
            }
            collectedJinjos = collectedBits(worldId);
            collectedJinjos |= jinjoBitFromActor(actorId);
            if (collectedJinjos == kAllJinjos) {
                ItemQueue::AddCheck(jinjoJiggyChecks[worldId]);
            }
            setCollectedBits(worldId, collectedJinjos);
            // Spawn vanilla jinjo on top of the player because the animation is too complex to handle without doing
            // this. Collection behaviour is canceled in Jinjo.cpp so that part can all be handled here in the
            // ItemQueue.
            switch (actorId) {
                case ACTOR_5E_JINJO_YELLOW:
                    actorInfo = &chJinjoYellow;
                    break;
                case ACTOR_5F_JINJO_ORANGE:
                    actorInfo = &chJinjoOrange;
                    break;
                case ACTOR_60_JINJO_BLUE:
                    actorInfo = &chJinjoBlue;
                    break;
                case ACTOR_61_JINJO_PINK:
                    actorInfo = &chJinjoPink;
                    break;
                case ACTOR_62_JINJO_GREEN:
                    actorInfo = &chJinjoGreen;
                    break;
            }
            player_getPosition_s32(playerPosition);
            customActor = actor_new(playerPosition, 0, actorInfo, ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_8);
            break;
        case RITYPE_MOLEHILL:
            coMusicPlayer_playMusic(COMUSIC_D_JINGLE_JIGGY_COLLECTED, -1);
            switch (randoItemId) {
                case RI_MOLEHILL_BARGE:
                    ability_unlock(ABILITY_0_BARGE);
                    break;
                case RI_MOLEHILL_BEAK_BOMB:
                    ability_unlock(ABILITY_1_BEAK_BOMB);
                    break;
                case RI_MOLEHILL_BEAK_BUSTER:
                    ability_unlock(ABILITY_2_BEAK_BUSTER);
                    break;
                case RI_MOLEHILL_CAMERA_CONTROL:
                    ability_unlock(ABILITY_3_CAMERA_CONTROL);
                    break;
                case RI_MOLEHILL_CLAW_SWIPE:
                    ability_unlock(ABILITY_4_CLAW_SWIPE);
                    ability_unlock(ABILITY_C_ROLL);
                    ability_unlock(ABILITY_B_RATATAT_RAP);
                    break;
                case RI_MOLEHILL_CLIMB:
                    ability_unlock(ABILITY_5_CLIMB);
                    break;
                case RI_MOLEHILL_DIVE:
                    ability_unlock(ABILITY_F_DIVE);
                    break;
                case RI_MOLEHILL_EGGS:
                    ability_unlock(ABILITY_6_EGGS);
                    item_adjustByDiffWithHud(ITEM_D_EGGS, 50);
                    break;
                case RI_MOLEHILL_FLAP_FLIP:
                    ability_unlock(ABILITY_A_HOLD_A_JUMP_HIGHER);
                    ability_unlock(ABILITY_7_FEATHERY_FLAP);
                    ability_unlock(ABILITY_8_FLAP_FLIP);
                    break;
                case RI_MOLEHILL_FLIGHT:
                    ability_unlock(ABILITY_9_FLIGHT);
                    item_adjustByDiffWithHud(ITEM_F_RED_FEATHER, 25);
                    break;
                case RI_MOLEHILL_SHOCK_JUMP:
                    ability_unlock(ABILITY_D_SHOCK_JUMP);
                    break;
                case RI_MOLEHILL_TALON_TROT:
                    ability_unlock(ABILITY_10_TALON_TROT);
                    break;
                case RI_MOLEHILL_TURBO_TALON:
                    ability_unlock(ABILITY_11_TURBO_TALON);
                    break;
                case RI_MOLEHILL_WADING_BOOTS:
                    ability_unlock(ABILITY_E_WADING_BOOTS);
                    break;
                case RI_MOLEHILL_WONDERWING:
                    ability_unlock(ABILITY_12_WONDERWING);
                    item_adjustByDiffWithHud(ITEM_10_GOLD_FEATHER, 5);
                    break;
                default:
                    break;
            }
            break;
        case RITYPE_MUMBO_TOKEN:
            func_8030E760((sfx_e)0x401, 1.0f, 0x7fff);
            for (uint32_t i = MUMBOTOKEN_01_MM_STUMP_NEAR_CONGA;
                 i <= MUMBOTOKEN_73_CCW_WINTER_SIR_SLUSH_BETWEEN_BIG_FLOWER_AND_MUMBOS_SKULL; i++) {
                if ((sMumboTokenScore[(i - 1) / 8] & (1 << (i & 7))) == 0) {
                    mumboscore_set((mumbotoken_e)i, true);
                    break;
                }
            }
            item_inc(ITEM_1C_MUMBO_TOKEN);
            break;
        case RITYPE_MUSIC_NOTE:
            coMusicPlayer_playMusic(COMUSIC_9_NOTE_COLLECTED, 16000);
            D_80385FF0[worldId]++;
            if (worldId == map_getLevel(gsworld_getMap())) {
                item_set(ITEM_C_NOTE, D_80385FF0[map_getLevel(gsworld_getMap())]);
            }
            break;
        case RITYPE_SNS_EGG:
            comusic_playTrack(COMUSIC_88_BIG_SNS_FANFARE);
            FUNC_8030E624(SFX_114_BRICKWALL_BREAKING, 0.8f, 15000);
            switch (randoItemId) {
                case RI_STOP_N_SWOP_EGG_BLUE:
                    sns_set_item_and_update_payload(SNS_ITEM_EGG_BLUE, 0, 1);
                    break;
                case RI_STOP_N_SWOP_EGG_CYAN:
                    sns_set_item_and_update_payload(SNS_ITEM_EGG_CYAN, 0, 1);
                    break;
                case RI_STOP_N_SWOP_EGG_GREEN:
                    sns_set_item_and_update_payload(SNS_ITEM_EGG_GREEN, 0, 1);
                    break;
                case RI_STOP_N_SWOP_EGG_PINK:
                    sns_set_item_and_update_payload(SNS_ITEM_EGG_PINK, 0, 1);
                    break;
                case RI_STOP_N_SWOP_EGG_RED:
                    sns_set_item_and_update_payload(SNS_ITEM_EGG_RED, 0, 1);
                    break;
                case RI_STOP_N_SWOP_EGG_YELLOW:
                    sns_set_item_and_update_payload(SNS_ITEM_EGG_YELLOW, 0, 1);
                    break;
            }
            break;
        case RITYPE_SNS_KEY:
            sns_set_item_and_update_payload(SNS_ITEM_ICE_KEY, 0, 1);
            comusic_playTrack(COMUSIC_88_BIG_SNS_FANFARE);
            break;
        case RITYPE_AP_ITEM:
            break;
        default:
            break;
    }
}

void ItemQueue::SendNotification(RandoItemId randoItemId) {
    if (CVAR_SHOW_COLLISION_NOTIFICATIONS) {
        RandoItemType itemType = Rando::StaticData::Items[randoItemId].randoItemType;
        actor_e actorId = (actor_e)Rando::StaticData::Items[randoItemId].actorId;
        std::string prefix = "";
        std::string message = Rando::StaticData::Items[randoItemId].name;
        std::string suffix = "";
        ImVec4 itemColor = UIWidgets::ColorValues.at(itemColors[actorId]);

        if (itemType == RITYPE_MOLEHILL) {
            prefix = "You learned";
        } else if (itemType == RITYPE_SNS_EGG || itemType == RITYPE_SNS_KEY) {
            int32_t totalsnsItems = Rando::Logic::GetTotalSnsItemsCollected();
            prefix = "You collected ";
            prefix += Rando::StaticData::Items[randoItemId].article;
            suffix = "(";
            suffix += std::to_string(totalsnsItems);
            suffix += " / 7)";

            itemColor = UIWidgets::ColorValues.at(snsColors[randoItemId]);
        } else {
            prefix = "You collected ";
            prefix += Rando::StaticData::Items[randoItemId].article;
        }

        Notification::Emit({
            .prefix = prefix,
            .prefixColor = UIWidgets::ColorValues.at(UIWidgets::Colors::White),
            .message = message,
            .messageColor = itemColor,
            .suffix = suffix,
            .suffixColor = UIWidgets::ColorValues.at(UIWidgets::Colors::White),
        });
    }
}

void ItemQueue::Clear() {
    while (!itemQueue.empty()) {
        itemQueue.pop();
    }
}

void ItemQueue::AddCheck(RandoCheckId randoCheckId) {
    RANDO_SAVE_CHECKS[randoCheckId].eligible = true;
    itemQueue.push(randoCheckId);
}

void RegisterItemQueue() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) { ItemQueue::Process(); });

    COND_HOOK(OnSaveFileLoad, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) { ItemQueue::Clear(); });
}

static RegisterShipInitFunc initFunc(RegisterItemQueue, { "IS_RANDO" });
