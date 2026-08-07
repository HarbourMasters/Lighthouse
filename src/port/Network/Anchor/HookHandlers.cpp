#include "Anchor.h"
#include "Authority.h"
#include "VileSync.h"
#include "FightSync.h"
#include <libultraship/libultraship.h>
//#include "soh/frame_interpolation.h"
#include "port/Engine.h"
#include "port/UI/Notification.h"
#include "port/Patches/Patches.h"
#include <unordered_set>

#include "functions.h"
extern "C" {
#include "variables.h"

float OTRGetDimensionFromLeftEdge(float v);
float OTRGetDimensionFromRightEdge(float v);
s32 chMrVile_netGetAnimMode(Actor* actor);
void port_jiggySpawn_remove(int32_t jiggyId);
int32_t port_mapFlag_wasSetRemotely(int32_t index);
bool __chSmBottles_isAnySpiralMountainAbilityLearned(void);
}

extern "C" void port_fpTwinkly_release(void) {
    NetAuthority_Release(NET_ACTIVITY_FP_TWINKLY);
}

s32 Anchor_LevelOfMap(s32 map);

// Every listener below that alters vanilla behaviour is wrapped in this. Presence-only rooms
// (the global room, or sync turned off) must play exactly like single player.
static bool Anchor_WorldSyncActive() {
    Anchor* anchor = Anchor::GetInstance();
    return anchor != nullptr && anchor->IsWorldSyncActive();
}

// True when a remote client owns the Mr. Vile minigame (our local logic must follow).
static bool Anchor_IsVileFollower() {
    return !NetAuthority_IsSelf(NET_ACTIVITY_VILE_MINIGAME);
}

// True when we are the live, connected authority for the Mr. Vile minigame.
static bool Anchor_IsVileAuthority() {
    return NetAuthority_IsClaimed(NET_ACTIVITY_VILE_MINIGAME) && NetAuthority_IsSelf(NET_ACTIVITY_VILE_MINIGAME);
}

// Authority-side per-frame work: stream Mr. Vile's transform and broadcast the periodic
// snapshot while the minigame is claimed.
static void Anchor_UpdateVileSync() {
    if (!Anchor_IsVileAuthority() || gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }

    f32 origin[3] = { 0.0f, 0.0f, 0.0f };
    f32 dist;
    Actor* vile = actorArray_findClosestActorFromActorId(origin, ACTOR_13A_MR_VILE, -1, &dist);
    if (vile != nullptr) {
        Anchor::GetInstance()->SendPacket_VileUpdate(vile->position, vile->pitch, vile->yaw, vile->roll,
                                                     (u8)chMrVile_netGetAnimMode(vile));
    }

    static u32 sSnapshotTimer = 0;
    if (++sSnapshotTimer >= 60) {
        sSnapshotTimer = 0;
        Anchor::GetInstance()->SendPacket_VileGameState();
    }
}

static void Anchor_UpdateFightSync() {
    auto* anchor = Anchor::GetInstance();
    if (!Anchor_WorldSyncActive() || !anchor->IsSaveLoaded() || gsworld_getMap() != MAP_90_GL_BATTLEMENTS) {
        return;
    }

    f32 pos[3];
    f32 yaw;
    s32 state, phase, mirror, vuln;
    if (!FightSync_GatherUpdate(pos, &yaw, &state, &phase, &mirror, &vuln)) {
        return;
    }

    if (!NetAuthority_IsClaimed(NET_ACTIVITY_FINAL_BOSS)) {
        NetAuthority_Claim(NET_ACTIVITY_FINAL_BOSS);
    }
    if (NetAuthority_IsClaimed(NET_ACTIVITY_FINAL_BOSS) && NetAuthority_IsSelf(NET_ACTIVITY_FINAL_BOSS)) {
        anchor->SendPacket_FightUpdate(pos, yaw, state, phase, mirror, vuln);
    }
}

static bool Anchor_ShouldBroadcastVolatileFlag(s32 index) {
    static const std::unordered_set<s32> syncList = {
        VOLATILE_FLAG_B6_WITCH_SWITCH_PRESSED_MM,  VOLATILE_FLAG_B7_WITCH_SWITCH_PRESSED_MMM,
        VOLATILE_FLAG_B8_WITCH_SWITCH_PRESSED_TTC, VOLATILE_FLAG_B9_WITCH_SWITCH_PRESSED_RBB,
        VOLATILE_FLAG_BA_WITCH_SWITCH_PRESSED_CCW, VOLATILE_FLAG_BB_WITCH_SWITCH_PRESSED_FP,
        VOLATILE_FLAG_BC_WITCH_SWITCH_PRESSED_CC,  VOLATILE_FLAG_BD_WITCH_SWITCH_PRESSED_BGS,
        VOLATILE_FLAG_BE_WITCH_SWITCH_PRESSED_GV,
    };
    return syncList.contains(index);
}

bool Anchor_ScopedFlagExcluded(s32 space, s32 index) {
    static const std::unordered_set<s32> excluded = {
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_5_TTC_UNKNOWN,            // TTC sandcastle drain
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE, // FP xmas-tree ice shatter
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_30_RBB_UNKNOWN, // RBB anchor/Snorkel chain cutscene
        // GV water-pyramid rise: reapplied live from JIGGY_42 in water_pyramidrot.c.
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_6_GV_UNKNOWN,
        (ANCHOR_FLAGSPACE_MAP_SPECIFIC << 16) | MM_SPECIFIC_FLAG_2_ORANGE_HAS_BEEN_RETURNED,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_1C_MM_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_1D_TTC_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_1E_CC_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_1F_BGS_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_20_FP_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_21_GV_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_22_MMM_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_23_RBB_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_24_CCW_OPEN,
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_3F_LAIR_GRUNTY_DOOR_OPEN,
    };
    if (excluded.contains((space << 16) | index)) {
        return true;
    }
    // BGS jiggy-switch timers (walkway=3, maze=0xC): kept local.
    if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC && gsworld_getMap() == MAP_D_BGS_BUBBLEGLOOP_SWAMP &&
        (index == 3 || index == 0xC)) {
        return true;
    }
    if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC && gsworld_getMap() == MAP_7_TTC_TREASURE_TROVE_COVE &&
        index <= TTC_SPECIFIC_FLAG_3_BLUBBER_SHOW_JIGGY_SPAWNED_TEXT_FLAG) {
        return true;
    }
    // FP xmas tree star minigame (flags 2/3): per-player, kept local; result syncs via ANCHOR_PUZZLE_FP_TREE_ICE.
    if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC && gsworld_getMap() == MAP_27_FP_FREEZEEZY_PEAK &&
        (index == 2 || index == 3)) {
        return true;
    }
    // FP bear cubs' presents-received flags: kept local; ANCHOR_PUZZLE_FP_PRESENTS syncs the result.
    if (space == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC &&
        (s32)map_getLevel(gsworld_getMap()) == (s32)LEVEL_5_FREEZEEZY_PEAK &&
        (index == LEVEL_FLAG_11_FP_UNKNOWN || index == LEVEL_FLAG_12_FP_UNKNOWN || index == LEVEL_FLAG_13_FP_UNKNOWN)) {
        return true;
    }
    // SM map flags are entirely Bottles-tutorial choreography;
    // syncing any of them runs someone else's state machine.
    if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC && gsworld_getMap() == MAP_1_SM_SPIRAL_MOUNTAIN) {
        return true;
    }
    return false;
}

static bool Anchor_ShouldSyncItemCount(s32 item, const RoomState& room) {
    switch (item) {
        // Progress counters, not consumables; the caller already gates on syncItemsAndFlags.
        case ITEM_1C_MUMBO_TOKEN:
        case ITEM_26_JIGGY_TOTAL:
            return true;
        case ITEM_D_EGGS:
        case ITEM_F_RED_FEATHER:
        case ITEM_10_GOLD_FEATHER:
            return room.shareConsumables != 0;
        default:
            return false;
    }
}

void Anchor::RegisterHooks() {

    // #region Hooks that are required for basic Anchor functionality

    // COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, isConnected, [&]() {
    //     SendPacket_UpdateClientState();

    //    if (IsSaveLoaded()) {
    //        RefreshClientActors();
    //    }
    //});

    COND_HOOK(OnMapLoad, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        auto ev = reinterpret_cast<OnMapLoad*>(event);
        if (ev->prevMap == MAP_91_FILE_SELECT && ev->nextMap != MAP_1E_CS_START_NINTENDO &&
            ev->nextMap != MAP_1F_CS_START_RAREWARE) {
            Anchor::GetInstance()->SendPacket_UpdateClientState();
        }
        Anchor::GetInstance()->ClearDummies();
        Anchor::GetInstance()->PopulateDummies((GameMap)ev->nextMap);
        Authority_OnSelfMapChanged(ev->nextMap);
        s32 prevLevel = Anchor_LevelOfMap((s32)ev->prevMap);
        if (prevLevel == 0 || prevLevel != Anchor_LevelOfMap((s32)ev->nextMap)) {
            Anchor::GetInstance()->SweepUnoccupiedLevelState((GameMap)ev->nextMap);
        }
        Anchor::GetInstance()->SendPacket_MapLoad((GameMap)ev->nextMap, ev->exit);
        // Anchor::GetInstance()->SendPacket_PlayerUpdate(true);

        auto* anchor = Anchor::GetInstance();
        if (Anchor_WorldSyncActive() && ev->nextMap != MAP_91_FILE_SELECT && ev->nextMap != MAP_1E_CS_START_NINTENDO &&
            ev->nextMap != MAP_1F_CS_START_RAREWARE) {
            anchor->SendPacket_RequestScopedState((GameMap)ev->nextMap);

            s32 enteredFlag = -1;
            switch (ev->nextMap) {
                case MAP_2_MM_MUMBOS_MOUNTAIN:
                    enteredFlag = FILEPROG_B0_HAS_ENTERED_MM;
                    break;
                case MAP_7_TTC_TREASURE_TROVE_COVE:
                    enteredFlag = FILEPROG_B2_HAS_ENTERED_TTC;
                    break;
                case MAP_B_CC_CLANKERS_CAVERN:
                    enteredFlag = FILEPROG_B8_HAS_ENTERED_CC;
                    break;
                case MAP_D_BGS_BUBBLEGLOOP_SWAMP:
                    enteredFlag = FILEPROG_B1_HAS_ENTERED_BGS;
                    break;
                case MAP_12_GV_GOBIS_VALLEY:
                    enteredFlag = FILEPROG_B3_HAS_ENTERED_GV;
                    break;
                case MAP_1B_MMM_MAD_MONSTER_MANSION:
                    enteredFlag = FILEPROG_B7_HAS_ENTERED_MMM;
                    break;
                case MAP_27_FP_FREEZEEZY_PEAK:
                    enteredFlag = FILEPROG_B6_HAS_ENTERED_FP;
                    break;
                case MAP_31_RBB_RUSTY_BUCKET_BAY:
                    enteredFlag = FILEPROG_B4_HAS_ENTERED_RBB;
                    break;
                case MAP_40_CCW_HUB:
                    enteredFlag = FILEPROG_B5_HAS_ENTERED_CCW;
                    break;
                default:
                    break;
            }
            if (enteredFlag >= 0 && fileProgressFlag_get((enum file_progress_e)enteredFlag)) {
                anchor->SendPacket_SetFlag((u8)ANCHOR_FLAGSPACE_FILE_PROGRESS, (s16)enteredFlag);
            }
        }
    });

    COND_HOOK(OnReset, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        Anchor::GetInstance()->SendPacket_MapLoad((GameMap)getDefaultBootMap(), gsworld_getExit());
    });

    COND_HOOK(OnGameLoad, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        anchor->hasCheckedRandoCompat = false;
        anchor->reloadMapOnTeamState = false;
        anchor->SendPacket_RequestTeamState(true);
    });

    COND_HOOK(OnPlayerDraw, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        auto drawEv = reinterpret_cast<OnPlayerDraw*>(event);
        Anchor::GetInstance()->DrawDummies(reinterpret_cast<OnPlayerDraw*>(drawEv));
    });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_HIGH, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        anchor->SendPacket_PlayerUpdate();
        anchor->ProcessIncomingPacketQueue();
        anchor->RefreshClientActors();
        anchor->UpdateDummies();
        Anchor_UpdateVileSync();
        Anchor_UpdateFightSync();

        if (Anchor_WorldSyncActive() && anchor->IsSaveLoaded()) {
            anchor->FlushPendingJiggySpawns();
            if (!anchor->hasCheckedRandoCompat) {
                anchor->CheckRandoRoomCompatibility();
                anchor->hasCheckedRandoCompat = true;
            }
        }
    });

    COND_HOOK(OnPlayerTransformChange, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto ev = reinterpret_cast<OnPlayerTransformChange*>(event);
        Anchor::GetInstance()->SendPacket_PlayerTransformChange(ev->tf_id);
    });

    // #region Mr. Vile minigame sync

    // Authority lifecycle: the client whose controller leaves idle claims the minigame;
    // returning to idle (or the player declining) releases it.
    COND_HOOK(OnVileGameStateChange, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto ev = reinterpret_cast<OnVileGameStateChange*>(event);
        if (!Anchor_WorldSyncActive() || gsworld_getMap() != MAP_10_BGS_MR_VILE) {
            return;
        }
        if (ev->state >= 2) {
            NetAuthority_Claim(NET_ACTIVITY_VILE_MINIGAME);
            // Push round start/end immediately rather than waiting for the periodic snapshot.
            if (Anchor_IsVileAuthority()) {
                Anchor::GetInstance()->SendPacket_VileGameState();
            }
        } else {
            NetAuthority_Release(NET_ACTIVITY_VILE_MINIGAME);
        }
    });

    // Authority broadcasts hole state changes (appear/hide/eaten).
    COND_HOOK(OnVileHoleStateChange, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto ev = reinterpret_cast<OnVileHoleStateChange*>(event);
        if (!Anchor_IsVileAuthority()) {
            return;
        }
        if (ev->state != 2 && ev->state != 4 && ev->state != 5) {
            return;
        }
        VileHoleId hole = VileHoles_IdFromPosition(ev->position[0], ev->position[2]);
        if (hole == VILE_HOLE_NONE) {
            return;
        }
        Anchor::GetInstance()->SendPacket_VileHoleState((u8)hole, (u8)ev->state, (u8)ev->pieceType, VILE_EATER_MR_VILE);
    });

    // Followers: suppress local random logic; network state drives these instead.
    COND_VB_SHOULD(VB_CCW_FLOWER_REMOTE_GROW, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive()) {
            s32 stageFlag = va_arg(args, s32);
            *should = fileProgressFlag_get((enum file_progress_e)stageFlag) != 0;
        }
    });

    COND_VB_SHOULD(VB_CC_RINGS_SNAP_WATER, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive()) {
            *should = false;
        }
    });

    // Vanilla despawns the CCW podium until its switch is pressed, and only rebuilds it when the cube
    // re-streams. Keep the actor alive (hidden) instead, so a teammate's press reveals it in place.
    COND_VB_SHOULD(VB_CCW_PODIUM_DESPAWN, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive()) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_JIGSAW_PICTURE_RESYNC, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive()) {
            *should = true;
        }
    });

    // Lair door remote-open: Door of Grunty's open flag (0xE2) is already set on arrival; key off
    // visual state (fully open == 0x1B) instead. Other lair doors stay flag-based.
    COND_VB_SHOULD(VB_LEVELDOOR_REMOTE_OPEN_DONE, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive()) {
            s32 doorActorId = va_arg(args, s32);
            s32 doorState = va_arg(args, s32);
            if (doorActorId == ACTOR_2E5_LARGE_DOOR_TO_FINAL_BATTLE) {
                *should = (doorState == 0x1B);
            }
        }
    });

    COND_VB_SHOULD(VB_FP_TWINKLY_START, EVENT_PRIORITY_NORMAL, isConnected, {
        if (NetAuthority_IsClaimed(NET_ACTIVITY_FP_TWINKLY) && !NetAuthority_IsSelf(NET_ACTIVITY_FP_TWINKLY)) {
            *should = false;
        } else {
            NetAuthority_Claim(NET_ACTIVITY_FP_TWINKLY);
        }
    });

    COND_VB_SHOULD(VB_DOOR_OPEN_CAMERA, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive()) {
            s32 doorId = va_arg(args, s32);
            switch (doorId) {
                case GV_DOOR_CAM_SUN:
                    *should = !port_mapFlag_wasSetRemotely(3);
                    break;
                case GV_DOOR_CAM_STAR:
                    *should = !port_mapFlag_wasSetRemotely(5);
                    break;
                case GV_DOOR_CAM_KAZOOIE:
                    *should = !port_mapFlag_wasSetRemotely(6);
                    break;
                case GV_DOOR_CAM_JINXY:
                    *should = !(port_mapFlag_wasSetRemotely(0) && port_mapFlag_wasSetRemotely(1));
                    break;
                case MMM_DOOR_CAM_CHURCH:
                    *should = !port_mapFlag_wasSetRemotely(0); // MMM_SPECIFIC_FLAG_0_UNKNOWN
                    break;
                default:
                    break;
            }
        }
    });

    COND_VB_SHOULD(VB_VILE_YUMBLIE_EMERGE, EVENT_PRIORITY_NORMAL, true, {
        if (Anchor_IsVileFollower()) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_VILE_YUMBLIE_HIDE, EVENT_PRIORITY_NORMAL, true, {
        if (Anchor_IsVileFollower()) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_VILE_GAME_UPDATE, EVENT_PRIORITY_NORMAL, true, {
        if (Anchor_IsVileFollower()) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_VILE_CPU_AI, EVENT_PRIORITY_NORMAL, true, {
        if (Anchor_IsVileFollower()) {
            *should = false;
        }
    });

    // Followers: a local chomp becomes an eat request; the authority validates and the
    // resulting eaten state comes back as a VILE_HOLE_STATE packet.
    COND_VB_SHOULD(VB_VILE_PLAYER_EAT_PIECE, EVENT_PRIORITY_NORMAL, true, {
        if (Anchor_IsVileFollower()) {
            f32* piecePos = va_arg(args, f32*);
            VileHoleId hole = VileHoles_IdFromPosition(piecePos[0], piecePos[2]);
            if (hole != VILE_HOLE_NONE) {
                Anchor::GetInstance()->SendPacket_VileEatRequest((u8)hole);
            }
            *should = false;
        }
    });

    // #endregion

    COND_HOOK(OnPlayerAnimReset, EVENT_PRIORITY_HIGH, true,
              [](IEvent* event) { Anchor::GetInstance()->SendPacket_PlayerAnimReset(); });

    COND_HOOK(OnPlayerAnimChange, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        OnPlayerAnimChange* ev = reinterpret_cast<OnPlayerAnimChange*>(event);
        Anchor::GetInstance()->SendPacket_PlayerAnimChange(ev->anim_id, ev->duration, ev->control, ev->start_position,
                                                           ev->subrange_end, ev->smooth);
    });

    COND_HOOK(OnPlayerAnimSubRangeChange, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        OnPlayerAnimSubRangeChange* ev = reinterpret_cast<OnPlayerAnimSubRangeChange*>(event);
        Anchor::GetInstance()->SendPacket_PlayerSubRangeChange(ev->duration, ev->end_position);
    });

    COND_HOOK(OnActorDestroy, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        OnActorDestroy* ev = reinterpret_cast<OnActorDestroy*>(event);
        Anchor::GetInstance()->OnActorDestroyed(ev->actor);
    });

    // #region Flag sync

    COND_HOOK(OnGameFlagSet, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnGameFlagSet*>(event);
        for (s32 i = 0; i < ev->length; i++) {
            s32 index = ev->index + i;
            if (ev->flagSpace == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC) {
                if (Anchor_ScopedFlagExcluded(ev->flagSpace, index)) {
                    continue;
                }
                anchor->SendPacket_ScopedFlag((u8)ev->flagSpace, (s16)index,
                                              (u8)(levelSpecificFlags_get(index) ? 1 : 0));
                continue;
            }
            if (ev->flagSpace == ANCHOR_FLAGSPACE_MAP_SPECIFIC) {
                if (Anchor_ScopedFlagExcluded(ev->flagSpace, index)) {
                    continue;
                }
                anchor->SendPacket_ScopedFlag((u8)ev->flagSpace, (s16)index, (u8)(mapSpecificFlags_get(index) ? 1 : 0));
                continue;
            }
            s32 bit;
            if (ev->flagSpace == ANCHOR_FLAGSPACE_VOLATILE) {
                if (!Anchor_ShouldBroadcastVolatileFlag(index)) {
                    continue;
                }
                bit = volatileFlag_get((enum volatile_flags_e)index);
            } else {
                bit = fileProgressFlag_get((enum file_progress_e)index);
            }
            if (bit) {
                anchor->SendPacket_SetFlag((u8)ev->flagSpace, (s16)index);
            } else {
                anchor->SendPacket_UnsetFlag((u8)ev->flagSpace, (s16)index);
            }
        }
    });

    COND_HOOK(OnItemCountChanged, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnItemCountChanged*>(event);
        if (Anchor_ShouldSyncItemCount(ev->item, anchor->roomState)) {
            anchor->SendPacket_SetItemCount((s16)ev->item, ev->count);
        }
    });

    COND_HOOK(OnAbilityLearned, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnAbilityLearned*>(event);
        anchor->SendPacket_SetAbility((s16)ev->move, (u8)ev->value);
    });

    COND_HOOK(OnCollectibleCollected, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnCollectibleCollected*>(event);
        anchor->SendPacket_CollectItem((u8)ev->kind, (s32)ev->id);
    });

    COND_HOOK(OnRandoCheckObtained, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnRandoCheckObtained*>(event);
        anchor->SendPacket_SetCheckStatus((s32)ev->randoCheckId, (s32)ev->map);
    });

    COND_HOOK(OnJiggySpawned, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnJiggySpawned*>(event);
        anchor->SendPacket_SpawnJiggy((s16)ev->jiggyId, ev->x, ev->y, ev->z);
    });

    COND_HOOK(OnHoneycombDropSpawn, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto ev = reinterpret_cast<OnHoneycombDropSpawn*>(event);
        Anchor::GetInstance()->SendPacket_SpawnHoneycomb((s16)ev->honeycombId, ev->bundleId, ev->x, ev->y, ev->z);
    });

    // SM intro Bottles: the tutorial offer is first-answer-wins for the team.
    COND_VB_SHOULD(VB_SM_TUTORIAL_CHOICE_OPEN, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive()) {
            // By level, not map: a romhack's Spiral Mountain may be a different map id.
            if (__chSmBottles_isAnySpiralMountainAbilityLearned() ||
                (port_puzzleStep_getForLevel(LEVEL_B_SPIRAL_MOUNTAIN, ANCHOR_PUZZLE_SM_TUTORIAL) & 1)) {
                *should = false;
            } else if (NetAuthority_IsClaimed(NET_ACTIVITY_SM_TUTORIAL) &&
                       !NetAuthority_IsSelf(NET_ACTIVITY_SM_TUTORIAL)) {
                *should = false;
            } else {
                NetAuthority_Claim(NET_ACTIVITY_SM_TUTORIAL);
            }
        }
    });

    // Ability molehills wake up only after the tutorial choice exists: the offer was
    // answered (synced bit) or a move is already known (covers skip-tutorial saves).
    COND_VB_SHOULD(VB_SM_MOLEHILL_ACTIVE, EVENT_PRIORITY_NORMAL, isConnected, {
        if (Anchor_WorldSyncActive() && !__chSmBottles_isAnySpiralMountainAbilityLearned() &&
            !(port_puzzleStep_getForLevel(LEVEL_B_SPIRAL_MOUNTAIN, ANCHOR_PUZZLE_SM_TUTORIAL) & 1)) {
            *should = false;
        }
    });

    COND_HOOK(OnTimedJiggyExpired, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        if (!Anchor_WorldSyncActive()) {
            return;
        }
        auto ev = reinterpret_cast<OnTimedJiggyExpired*>(event);
        port_jiggySpawn_remove(ev->jiggyId);
    });

    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_NORMAL, isConnected,
              [](IEvent* event) { Anchor::GetInstance()->SendPacket_UpdateTeamState(); });

    // #endregion

    //    COND_HOOK(OnPlayerSfx, isConnected, [&](u16 sfxId) { SendPacket_PlayerSfx(sfxId); });
    //
    //    COND_HOOK(OnRandoSetCheckStatus, isConnected, [&](RandomizerCheck rc, RandomizerCheckStatus status) {
    //        if (!isHandlingUpdateTeamState) {
    //            SendPacket_SetCheckStatus(rc);
    //        }
    //    });
    //
    //    COND_HOOK(OnRandoSetIsSkipped, isConnected, [&](RandomizerCheck rc, bool isSkipped) {
    //        if (!isHandlingUpdateTeamState) {
    //            SendPacket_SetCheckStatus(rc);
    //        }
    //    });
    //
    //    COND_HOOK(OnRandoEntranceDiscovered, isConnected,
    //              [&](u16 entranceIndex, u8 isReversedEntrance) { SendPacket_EntranceDiscovered(entranceIndex); });
    //
    //    COND_ID_HOOK(OnBossDefeat, ACTOR_BOSS_GANON2, isConnected, [&](void* refActor) { SendPacket_GameComplete();
    //    });
    //
    //    COND_HOOK(OnItemReceive, isConnected, [&](GetItemEntry itemEntry) {
    //        // Handle vanilla dungeon items a bit differently
    //        if (itemEntry.modIndex == MOD_NONE &&
    //            (itemEntry.itemId >= ITEM_KEY_BOSS && itemEntry.itemId <= ITEM_KEY_SMALL)) {
    //            SendPacket_UpdateDungeonItems();
    //            return;
    //        }
    //
    //        SendPacket_GiveItem(itemEntry.tableId, itemEntry.getItemId);
    //    });
    //
    //    // #endregion
}
