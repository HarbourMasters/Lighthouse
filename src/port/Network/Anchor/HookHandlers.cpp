#include "Anchor.h"
#include <libultraship/libultraship.h>
//#include "soh/frame_interpolation.h"
#include "port/Engine.h"

extern "C" {
#include "variables.h"
#include "functions.h"

float OTRGetDimensionFromLeftEdge(float v);
float OTRGetDimensionFromRightEdge(float v);
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
        Anchor::GetInstance()->PopulateDummies();
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

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        static bool sendUpdate = true;
        Anchor::GetInstance()->SendPacket_PlayerUpdate();
        Anchor::GetInstance()->ProcessIncomingPacketQueue();
        Anchor::GetInstance()->RefreshClientActors();
        Anchor::GetInstance()->UpdateDummies();
    });

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
