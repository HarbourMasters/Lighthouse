#pragma once

#include <libultraship/bridge/eventsbridge.h>

DEFINE_EVENT(OnGameSave, int32_t fileNum;)

DEFINE_EVENT(OnGameLoad, int32_t fileNum;)

DEFINE_EVENT(OnSaveFileLoad, int32_t fileNum; void* saveBuffer; int32_t result;)

DEFINE_EVENT(OnSaveFileSave, void* saveBuffer; int32_t fileNum; int32_t * result;)

DEFINE_EVENT(MapTransitionEnd, GameMap map; s32 exitID;);

DEFINE_EVENT(OnActorDestroy, Actor* actor;);

DEFINE_EVENT(OnPropInit, Prop* propPtr;);