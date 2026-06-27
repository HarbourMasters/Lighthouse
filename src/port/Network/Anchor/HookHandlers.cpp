#include "Anchor.h"
#include "Authority.h"
#include "VileSync.h"
#include <libultraship/libultraship.h>
//#include "soh/frame_interpolation.h"
#include "port/Engine.h"
#include <unordered_set>

extern "C" {
#include "variables.h"
#include "functions.h"

float OTRGetDimensionFromLeftEdge(float v);
float OTRGetDimensionFromRightEdge(float v);
s32 chvile_netGetAnimMode(Actor* actor);
}

// True when a remote client owns the Mr. Vile minigame (our local logic must follow).
static bool Anchor_IsVileFollower() {
    return !NetAuthority_IsSelf(NET_ACTIVITY_VILE_MINIGAME);
}

// True when we are the live, connected authority for the Mr. Vile minigame.
static bool Anchor_IsVileAuthority() {
    return Anchor::GetInstance()->isConnected && NetAuthority_IsClaimed(NET_ACTIVITY_VILE_MINIGAME) &&
           NetAuthority_IsSelf(NET_ACTIVITY_VILE_MINIGAME);
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
                                                     (u8)chvile_netGetAnimMode(vile));
    }

    static u32 sSnapshotTimer = 0;
    if (++sSnapshotTimer >= 60) {
        sSnapshotTimer = 0;
        Anchor::GetInstance()->SendPacket_VileGameState();
    }
}

// Volatile flags broadcast individually unless listed here (high-churn / per-frame).
static bool Anchor_ShouldBroadcastVolatileFlag(s32 index) {
    static const std::unordered_set<s32> syncList = {
        VOLATILE_FLAG_B6_WITCH_SWITCH_PRESSED_MM,
        VOLATILE_FLAG_B7_WITCH_SWITCH_PRESSED_MMM,
        VOLATILE_FLAG_B8_WITCH_SWITCH_PRESSED_TTC,
        VOLATILE_FLAG_B9_WITCH_SWITCH_PRESSED_RBB,
        VOLATILE_FLAG_BA_WITCH_SWITCH_PRESSED_CCW,
        VOLATILE_FLAG_BB_WITCH_SWITCH_PRESSED_FP,
        VOLATILE_FLAG_BC_WITCH_SWITCH_PRESSED_CC,
        VOLATILE_FLAG_BD_WITCH_SWITCH_PRESSED_BGS,
        VOLATILE_FLAG_BE_WITCH_SWITCH_PRESSED_GV,
    };
    return syncList.contains(index);
}

// Curated exclude-list for scoped (level/map) flags that shouldn't broadcast — flags with
// per-client consume semantics not covered by getClear. Keyed (space << 16) | index. Applied to
// both the realtime broadcast (below) and the entry-sync (ScopedState.cpp), so these flags stay
// strictly local to each client.
bool Anchor_ScopedFlagExcluded(s32 space, s32 index) {
    static const std::unordered_set<s32> excluded = {
        // One-shot cutscene triggers: each client plays its own cutscene when it personally
        // completes the puzzle, so sharing the flag replays the cutscene for teammates who
        // already saw it. Persistent completion is tracked separately (LEVEL_FLAG_2 for TTC's
        // drained water, FILEPROG_13 for FP's twinkly minigame), which still syncs.
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_5_TTC_UNKNOWN,            // TTC sandcastle drain
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE, // FP xmas-tree ice shatter
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_30_RBB_UNKNOWN,           // RBB anchor/Snorkel chain cutscene
        // GV water-pyramid rise: transient cross-map cutscene handoff. Must stay local or it yanks a
        // teammate into the rise cutscene + map warp; the raised state is reapplied live from
        // JIGGY_42 in water_pyramidrot.c instead.
        (ANCHOR_FLAGSPACE_LEVEL_SPECIFIC << 16) | LEVEL_FLAG_6_GV_UNKNOWN,
    };
    return excluded.contains((space << 16) | index);
}

// Which spendable item counts sync in realtime. Mumbo tokens + jiggy total always; eggs and
// feathers only when the room shares consumables.
static bool Anchor_ShouldSyncItemCount(s32 item, const RoomState& room) {
    switch (item) {
        case ITEM_1C_MUMBO_TOKEN:
        case ITEM_26_JIGGY_TOTAL:
        // CCW carried collectibles (ITEM_22 worms, ITEM_23 acorns) are intentionally NOT synced
        // here. They're a shared pool mutated concurrently (one player collects while another
        // feeds), and absolute last-writer-wins drops one of a simultaneous +1/-1. Instead the
        // COLLECT_ITEM packet carries the delta: +1 on collect (id >= 0) and -1 on spend (id < 0),
        // which compose correctly. See CollectItem.cpp.
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
        Anchor::GetInstance()->SendPacket_MapLoad((GameMap)ev->nextMap, ev->exit);
        // Anchor::GetInstance()->SendPacket_PlayerUpdate(true);

        // Entry-sync: pull current level/map scoped flags from teammates already there.
        auto* anchor = Anchor::GetInstance();
        if (anchor->isConnected && anchor->roomState.syncItemsAndFlags &&
            ev->nextMap != MAP_91_FILE_SELECT && ev->nextMap != MAP_1E_CS_START_NINTENDO &&
            ev->nextMap != MAP_1F_CS_START_RAREWARE) {
            anchor->SendPacket_RequestScopedState((GameMap)ev->nextMap);
        }
    });

    COND_HOOK(OnReset, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        Anchor::GetInstance()->SendPacket_MapLoad((GameMap)getDefaultBootMap(), gsworld_getExit());
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

        // Pull team state once per loaded-save session. OnMapLoad is too early — gsworld
        // flips the map after the event, so IsSaveLoaded() is still false there.
        if (anchor->isConnected && anchor->IsSaveLoaded()) {
            // Spawn any jiggies a teammate spawned in this map while we were elsewhere.
            anchor->FlushPendingJiggySpawns();
            if (!anchor->hasRequestedTeamState) {
                anchor->SendPacket_RequestTeamState();
                anchor->hasRequestedTeamState = true;
            }
        } else {
            anchor->hasRequestedTeamState = false;
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
        if (!Anchor::GetInstance()->isConnected || gsworld_getMap() != MAP_10_BGS_MR_VILE) {
            return;
        }
        if (ev->state >= 2) {
            NetAuthority_Claim(NET_ACTIVITY_VILE_MINIGAME);
            // Push the transition immediately (round start, round end with its result
            // jingle) instead of waiting for the next periodic snapshot. Gather rejects
            // the non-broadcast states, so this is a no-op for dialog transitions.
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

    // Broadcast individual flag changes (both spaces). Remote applies use the *_setEx
    // setters with triggerEvent=0, so this never fires for them (no echo).
    COND_HOOK(OnGameFlagSet, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnGameFlagSet*>(event);
        for (s32 i = 0; i < ev->length; i++) {
            s32 index = ev->index + i;
            // Transient level/map flags: scoped to same-level/same-map teammates, not queued.
            if (ev->flagSpace == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC) {
                if (Anchor_ScopedFlagExcluded(ev->flagSpace, index)) {
                    continue;
                }
                anchor->SendPacket_ScopedFlag((u8)ev->flagSpace, (s16)index, (u8)(levelSpecificFlags_get(index) ? 1 : 0));
                continue;
            }
            if (ev->flagSpace == ANCHOR_FLAGSPACE_MAP_SPECIFIC) {
                if (Anchor_ScopedFlagExcluded(ev->flagSpace, index)) {
                    continue;
                }
                anchor->SendPacket_ScopedFlag((u8)ev->flagSpace, (s16)index, (u8)(mapSpecificFlags_get(index) ? 1 : 0));
                continue;
            }
            // Persistent flags: team-wide, queued for offline teammates.
            s32 bit;
            if (ev->flagSpace == ANCHOR_FLAGSPACE_VOLATILE) {
                if (!Anchor_ShouldBroadcastVolatileFlag(index)) {
                    continue;
                }
                bit = volatileFlag_get((enum volatile_flags_e)index);
            } else {
                bit = fileProgressFlag_get((enum file_progress_e)index);
            }
            SPDLOG_INFO("[Anchor][flagdiag] broadcast space={} index={:#x} bit={}", ev->flagSpace, index, bit);
            if (bit) {
                anchor->SendPacket_SetFlag((u8)ev->flagSpace, (s16)index);
            } else {
                anchor->SendPacket_UnsetFlag((u8)ev->flagSpace, (s16)index);
            }
        }
    });

    // Realtime spendable item counts (absolute). Remote applies use item_setEx(triggerEvent=0),
    // so this never fires for them (no echo).
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

    // Realtime learned-move sync. Remote applies use ability_setLearnedEx(triggerEvent=0),
    // so this never fires for them (no echo).
    COND_HOOK(OnAbilityLearned, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnAbilityLearned*>(event);
        anchor->SendPacket_SetAbility((s16)ev->move, (u8)ev->value);
    });

    // Realtime collectible pickups (jiggy/honeycomb/Mumbo token) for live despawn + bit credit.
    COND_HOOK(OnCollectibleCollected, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnCollectibleCollected*>(event);
        anchor->SendPacket_CollectItem((u8)ev->kind, (s32)ev->id);
    });

    // Realtime jiggy spawns (witch switch, minigame reward) for same-map teammates.
    COND_HOOK(OnJiggySpawned, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        auto* anchor = Anchor::GetInstance();
        if (!anchor->IsSaveLoaded() || !anchor->roomState.syncItemsAndFlags) {
            return;
        }
        auto ev = reinterpret_cast<OnJiggySpawned*>(event);
        anchor->SendPacket_SpawnJiggy((s16)ev->jiggyId, ev->x, ev->y, ev->z);
    });

    // Push the full flag state to teammates on save.
    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_NORMAL, isConnected, [](IEvent* event) {
        Anchor::GetInstance()->SendPacket_UpdateTeamState();
    });

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
