#include "Anchor.h"
#include "Authority.h"
#include "VileSync.h"
#include <libultraship/libultraship.h>
//#include "soh/frame_interpolation.h"
#include "port/Engine.h"

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
    });

    COND_HOOK(OnReset, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        Anchor::GetInstance()->SendPacket_MapLoad((GameMap)getDefaultBootMap(), gsworld_getExit());
    });

    COND_HOOK(OnPlayerDraw, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        auto drawEv = reinterpret_cast<OnPlayerDraw*>(event);
        Anchor::GetInstance()->DrawDummies(reinterpret_cast<OnPlayerDraw*>(drawEv));
    });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_HIGH, isConnected, [](IEvent* event) {
        Anchor::GetInstance()->SendPacket_PlayerUpdate();
        Anchor::GetInstance()->ProcessIncomingPacketQueue();
        Anchor::GetInstance()->RefreshClientActors();
        Anchor::GetInstance()->UpdateDummies();
        Anchor_UpdateVileSync();
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

    //    COND_HOOK(OnPlayerSfx, isConnected, [&](u16 sfxId) { SendPacket_PlayerSfx(sfxId); });
    //    COND_HOOK(OnOcarinaNote, isConnected,
    //              [&](uint8_t note, float modulator, int8_t bend) { SendPacket_OcarinaSfx(note, modulator, bend); });
    //
    //    COND_HOOK(OnLoadGame, isConnected, [&](s16 fileNum) { justLoadedSave = true; });
    //
    //    COND_HOOK(OnSaveFile, isConnected, [&](s16 fileNum, int sectionID) {
    //        if (sectionID == 0) {
    //            SendPacket_UpdateTeamState();
    //        }
    //    });
    //
    //    COND_HOOK(OnFlagSet, isConnected,
    //              [&](s16 flagType, s16 flag) { SendPacket_SetFlag(SCENE_ID_MAX, flagType, flag); });
    //
    //    COND_HOOK(OnFlagUnset, isConnected,
    //              [&](s16 flagType, s16 flag) { SendPacket_UnsetFlag(SCENE_ID_MAX, flagType, flag); });
    //
    //    COND_HOOK(OnSceneFlagSet, isConnected,
    //              [&](s16 sceneNum, s16 flagType, s16 flag) { SendPacket_SetFlag(sceneNum, flagType, flag); });
    //
    //    COND_HOOK(OnSceneFlagUnset, isConnected,
    //              [&](s16 sceneNum, s16 flagType, s16 flag) { SendPacket_UnsetFlag(sceneNum, flagType, flag); });
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
