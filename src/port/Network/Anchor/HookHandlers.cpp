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
    //
    //    COND_HOOK(OnPresentFileSelect, isConnected, [&]() { SendPacket_UpdateClientState(); });
    //
    //    COND_ID_HOOK(ShouldActorInit, ACTOR_PLAYER, isConnected, [&](void* actorRef, bool* should) {
    //        Actor* actor = (Actor*)actorRef;
    //
    //        if (spawningDummyPlayerForClientId != 0) {
    //            SetDummyPlayerClientId(actor, spawningDummyPlayerForClientId);
    //
    //            // By the time we get here, the actor was already added to the ACTORCAT_PLAYER list, so we need to
    //            move it Actor_ChangeCategory(gPlayState, &gPlayState->actorCtx, actor, ACTORCAT_NPC); actor->id =
    //            ACTOR_EN_OE2; actor->category = ACTORCAT_NPC; actor->init = DummyPlayer_Init; actor->update =
    //            DummyPlayer_Update; actor->draw = DummyPlayer_Draw; actor->destroy = DummyPlayer_Destroy;
    //        }
    //    });
    //
    //    COND_HOOK(OnPlayerUpdate, isConnected, [&]() {
    //        if (justLoadedSave) {
    //            justLoadedSave = false;
    //            SendPacket_RequestTeamState();
    //        }
    //
    //        if (shouldRefreshActors) {
    //            shouldRefreshActors = false;
    //            RefreshClientActors();
    //        }
    //
    //        SendPacket_PlayerUpdate();
    //    });
    //
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
    //
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
    //    COND_HOOK(OnDungeonKeyUsed, isConnected, [&](uint16_t mapIndex) {
    //        // Handle vanilla dungeon items a bit differently
    //        SendPacket_UpdateDungeonItems();
    //    });
    //
    //    COND_VB_SHOULD(VB_APPLY_TUNIC_COLOR, isConnected, {
    //        Actor* myPlayer = (Actor*)GET_PLAYER(gPlayState);
    //        Actor* actor = va_arg(args, Actor*);
    //        Color_RGB8* color = va_arg(args, Color_RGB8*);
    //
    //        if (actor == myPlayer) {
    //            Color_RGBA8 ownColor = CVarGetColor(CVAR_REMOTE_ANCHOR("Color.Value"), { 100, 255, 100 });
    //            color->r = ownColor.r;
    //            color->g = ownColor.g;
    //            color->b = ownColor.b;
    //            return;
    //        }
    //
    //        uint32_t clientId = Anchor::GetInstance()->GetDummyPlayerClientId(actor);
    //
    //        if (!Anchor::GetInstance()->clients.contains(clientId)) {
    //            return;
    //        }
    //
    //        AnchorClient& client = Anchor::GetInstance()->clients[clientId];
    //        color->r = client.color.r;
    //        color->g = client.color.g;
    //        color->b = client.color.b;
    //    });
    //
    //    // #endregion
    //
    //    // #region Hooks for visual effects that don't affect gameplay
    //
    //    struct CompassIcon {
    //        Vec3f pos;
    //        Vec3s rot;
    //        float scale;
    //        Color_RGB8 color;
    //    };
    //
    //    COND_HOOK(OnMinimapDrawCompassIcons, isConnected, [&]() {
    //        if (!CVarGetInteger(CVAR_REMOTE_ANCHOR("ShowOtherPlayersOnMinimap"), 1) ||
    //            Anchor::GetInstance()->roomState.showLocationsMode == 0) {
    //            return;
    //        }
    //
    //        std::vector<CompassIcon> compassIcons;
    //
    //        bool isInDungeon = gPlayState->sceneNum == SCENE_DEKU_TREE || gPlayState->sceneNum ==
    //        SCENE_DODONGOS_CAVERN ||
    //                           gPlayState->sceneNum == SCENE_JABU_JABU || gPlayState->sceneNum == SCENE_FOREST_TEMPLE
    //                           || gPlayState->sceneNum == SCENE_FIRE_TEMPLE || gPlayState->sceneNum ==
    //                           SCENE_WATER_TEMPLE || gPlayState->sceneNum == SCENE_SPIRIT_TEMPLE ||
    //                           gPlayState->sceneNum == SCENE_SHADOW_TEMPLE || gPlayState->sceneNum ==
    //                           SCENE_BOTTOM_OF_THE_WELL || gPlayState->sceneNum == SCENE_ICE_CAVERN;
    //        std::string teamId = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    //
    //        // When transitioning to a new room via a door, curRoom.num updates immediately but the minimap still
    //        shows the
    //        // previous room while fading out
    //        s8 displayedRoomNum =
    //            gPlayState->roomCtx.prevRoom.num >= 0 ? gPlayState->roomCtx.prevRoom.num :
    //            gPlayState->roomCtx.curRoom.num;
    //
    //        for (auto& [clientId, client] : Anchor::GetInstance()->clients) {
    //            // Show compass icons for other players in the current scene. Also require them to be in the current
    //            room
    //            // within dungeons. If showLocationsMode isn't all players (2), only show compass icons for players of
    //            the
    //            // same team
    //            if (!client.self && client.online && client.player && client.sceneNum == gPlayState->sceneNum &&
    //                (!isInDungeon || client.curRoomNum == displayedRoomNum) &&
    //                (Anchor::GetInstance()->roomState.showLocationsMode == 2 || client.teamId == teamId)) {
    //                compassIcons.push_back(
    //                    CompassIcon{ client.player->actor.world.pos, client.player->actor.shape.rot, 0.3f,
    //                    client.color });
    //            }
    //        }
    //
    //        // The local player's compass icon is always last so it gets drawn above the others
    //        Player* player = GET_PLAYER(gPlayState);
    //        compassIcons.push_back(CompassIcon{ player->actor.world.pos, player->actor.shape.rot, 0.4f,
    //                                            CVarGetColor24(CVAR_REMOTE_ANCHOR("Color.Value"), { 100, 255, 100 })
    //                                            });
    //
    //        // Adapted internals of Minimap_DrawCompassIcons()
    //        s16 leftMinimapMargin = CVarGetInteger(CVAR_COSMETIC("HUD.Margin.L"), 0);
    //        s16 rightMinimapMargin = CVarGetInteger(CVAR_COSMETIC("HUD.Margin.R"), 0);
    //        s16 bottomMinimapMargin = CVarGetInteger(CVAR_COSMETIC("HUD.Margin.B"), 0);
    //
    //        s16 xMarginsMinimap;
    //        s16 yMarginsMinimap;
    //        if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.UseMargins"), 0) != 0) {
    //            if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosType"), 0) == ORIGINAL_LOCATION) {
    //                xMarginsMinimap = rightMinimapMargin;
    //            }
    //            yMarginsMinimap = bottomMinimapMargin;
    //        } else {
    //            xMarginsMinimap = 0;
    //            yMarginsMinimap = 0;
    //        }
    //
    //        s16 mapWidth = isInDungeon ? R_DGN_MINIMAP_X : R_OW_MINIMAP_X;
    //        s16 mapStartPosX = isInDungeon ? 96 : gMapData->owMinimapWidth[R_MAP_INDEX];
    //
    //        OPEN_DISPS(gPlayState->state.gfxCtx);
    //        Gfx_SetupDL_42Overlay(gPlayState->state.gfxCtx);
    //
    //        for (auto& compassIcon : compassIcons) {
    //            gSPMatrix(OVERLAY_DISP++, &gMtxClear, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    //            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE,
    //            0,
    //                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    //            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
    //            gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    //
    //            // The compass offset value is a factor of 10 compared to N64 screen pixels and originates in the
    //            center of
    //            // the screen Compute the additional mirror offset value by normalizing the original offset position
    //            and
    //            // taking it's distance to the center of the map, duplicating that result and casting back to a factor
    //            of 10 s16 mirrorOffset =
    //                ((mapWidth / 2) - ((R_COMPASS_OFFSET_X / 10) - (mapStartPosX - SCREEN_WIDTH / 2))) * 2 * 10;
    //
    //            s16 tempX = (s16)compassIcon.pos.x;
    //            s16 tempZ = (s16)compassIcon.pos.z;
    //            tempX /= R_COMPASS_SCALE_X * (CVarGetInteger(CVAR_ENHANCEMENT("MirroredWorld"), 0) ? -1 : 1);
    //            tempZ /= R_COMPASS_SCALE_Y;
    //
    //            s16 tempXOffset =
    //                R_COMPASS_OFFSET_X + (CVarGetInteger(CVAR_ENHANCEMENT("MirroredWorld"), 0) ? mirrorOffset : 0);
    //            if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosType"), 0) != ORIGINAL_LOCATION) {
    //                if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosType"), 0) == ANCHOR_LEFT) {
    //                    if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.UseMargins"), 0) != 0) {
    //                        xMarginsMinimap = leftMinimapMargin;
    //                    };
    //                    Matrix_Translate(
    //                        OTRGetDimensionFromLeftEdge((tempXOffset + (xMarginsMinimap * 10) + tempX +
    //                                                     (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosX"), 0) * 10))
    //                                                     /
    //                                                    10.0f),
    //                        (R_COMPASS_OFFSET_Y + ((yMarginsMinimap * 10) * -1) - tempZ +
    //                         ((CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosY"), 0) * 10) * -1)) /
    //                            10.0f,
    //                        0.0f, MTXMODE_NEW);
    //                } else if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosType"), 0) == ANCHOR_RIGHT) {
    //                    if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.UseMargins"), 0) != 0) {
    //                        xMarginsMinimap = rightMinimapMargin;
    //                    };
    //                    Matrix_Translate(
    //                        OTRGetDimensionFromRightEdge((tempXOffset + (xMarginsMinimap * 10) + tempX +
    //                                                      (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosX"), 0) * 10))
    //                                                      /
    //                                                     10.0f),
    //                        (R_COMPASS_OFFSET_Y + ((yMarginsMinimap * 10) * -1) - tempZ +
    //                         ((CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosY"), 0) * 10) * -1)) /
    //                            10.0f,
    //                        0.0f, MTXMODE_NEW);
    //                } else if (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosType"), 0) == ANCHOR_NONE) {
    //                    Matrix_Translate(
    //                        (tempXOffset + tempX + (CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosX"), 0) * 10)
    //                        / 10.0f), (R_COMPASS_OFFSET_Y + ((yMarginsMinimap * 10) * -1) - tempZ +
    //                         ((CVarGetInteger(CVAR_COSMETIC("HUD.Minimap.PosY"), 0) * 10) * -1)) /
    //                            10.0f,
    //                        0.0f, MTXMODE_NEW);
    //                }
    //            } else {
    //                Matrix_Translate(OTRGetDimensionFromRightEdge((tempXOffset + (xMarginsMinimap * 10) + tempX)
    //                / 10.0f),
    //                                 (R_COMPASS_OFFSET_Y + ((yMarginsMinimap * 10) * -1) - tempZ) / 10.0f, 0.0f,
    //                                 MTXMODE_NEW);
    //            }
    //            Matrix_Scale(compassIcon.scale, compassIcon.scale, compassIcon.scale, MTXMODE_APPLY);
    //            Matrix_RotateX(-1.6f, MTXMODE_APPLY);
    //            s16 rotation = ((0x7FFF - compassIcon.rot.y) / 0x400) *
    //                           (CVarGetInteger(CVAR_ENHANCEMENT("MirroredWorld"), 0) ? -1 : 1);
    //            Matrix_RotateY(rotation / 10.0f, MTXMODE_APPLY);
    //            gSPMatrix(OVERLAY_DISP++, MATRIX_NEWMTX(gPlayState->state.gfxCtx),
    //                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    //
    //            gDPSetPrimColor(OVERLAY_DISP++, 0, 0xFF, compassIcon.color.r, compassIcon.color.g,
    //            compassIcon.color.b,
    //                            255);
    //            gSPDisplayList(OVERLAY_DISP++, (Gfx*)gCompassArrowDL);
    //        }
    //
    //        CLOSE_DISPS(gPlayState->state.gfxCtx);
    //    });
    //
    //    // #endregion
}
