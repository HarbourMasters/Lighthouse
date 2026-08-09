#pragma once

extern "C" {
#include "prop.h"
}

struct StealthNoiseConfig {
    int map;
    float zoneMin;
    float zoneMax;
    float innerMin;
    float innerMax;
    int caughtTextId;
    int escapeWarpIndex;
    void (*escapeWarpFallback)(NodeProp*, ActorMarker*);
    int firstCatchToken;
};

void StealthNoise_Enable(const StealthNoiseConfig& cfg);
void StealthNoise_AddBurst(float amount, float seconds);
