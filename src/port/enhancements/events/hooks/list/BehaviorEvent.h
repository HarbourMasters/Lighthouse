#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>

#include "prop.h"

DEFINE_EVENT(OnBeakSwimVelocitySet, f32* velocity;)

DEFINE_EVENT(OnBoggyRaceSetSpeed, f32* speed;)

DEFINE_EVENT(OnFurnaceFunDialog, s32* lifeThreshold;)

DEFINE_EVENT(OnGruntyJinjonatorComplete)

DEFINE_EVENT(OnMumboTokenUpdate, Actor* actor;)

DEFINE_EVENT(OnWaterPyramidTimer, s32* timer;)

DEFINE_EVENT(OnBootLogosCheck, bool* skipLogos;)

DEFINE_EVENT(OnIntroCutsceneCheck, bool* skipIntro;)
