#include "ItemQueue.h"

#include <queue>
#include <unordered_map>
#include <map>

#include <libultraship/bridge.h>
#include "port/ShipInit.hpp"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CheckTracker/CheckTracker.h"
#include "port/Rando/Helpers/Helpers.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Prefs/Sections/RandoPrefs.h"

extern "C" {
#include "actor.h"
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

#define JIGGY_ID_MULTIPLIER(levelId) (1 + (10 * (levelId - 1)))
#define HONEYCOMB_ID_MULTIPLIER(levelId) (1 + (2 * (levelId - 1)))

static std::queue<RandoCheckId> itemQueue;

void ItemQueue::Process() {
    if (itemQueue.size() < 1) {
        return;
    }

    RandoCheckId randoCheckId = itemQueue.front();

    // Grant item
    RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
    if (!randoSaveCheck.received) {
        ItemQueue::GiveItem(randoSaveCheck.randoItemId);
        if (Prefs::Rando::Notifications) {
            Rando::Helpers::SendNotification(randoSaveCheck.randoItemId, "You");
        }
        RANDO_SAVE_CHECKS[randoCheckId].received = true;
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
                ItemQueue::AddCheck(Rando::StaticData::JinjoJiggyChecks[worldId]);
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

void ItemQueue::Clear() {
    while (!itemQueue.empty()) {
        itemQueue.pop();
    }
}

void ItemQueue::AddCheck(RandoCheckId randoCheckId) {
    RANDO_SAVE_CHECKS[randoCheckId].eligible = true;
    itemQueue.push(randoCheckId);
    CheckTracker_AddToCheckCount((uint32_t)randoCheckId);
}

void ItemQueue::RequeueMissedItems() {
    for (auto& check : RANDO_SAVE_CHECKS) {
        if (check.eligible && !check.received) {
            ItemQueue::AddCheck(check.randoCheckId);
        }
    }
}

void RegisterItemQueue() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) { ItemQueue::Process(); });

    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        ItemQueue::Clear();
        ItemQueue::RequeueMissedItems();
    });
}

static RegisterShipInitFunc initFunc(RegisterItemQueue, { "IS_RANDO" });
