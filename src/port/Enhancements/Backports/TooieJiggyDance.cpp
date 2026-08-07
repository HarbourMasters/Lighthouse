// Banjo-Tooie jiggy collect animation
// Created by wedarobi for Banjo Recomp
// Ported to Lighthouse

#include <cmath>

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Rando/Rando.h"

#include "functions.h"
extern "C" {
#include "enums.h"
#include "core2/particle.h"

ParticleEmitter* __fxSparkle_create(s16 position[3], f32 height, enum asset_e sprite_id);
}

#define CVAR_TOOIE_JIGGY_DANCE CVAR_ENHANCEMENT("Backports.JiggyAnimation")
#define CVAR_SKIP_JIGGY_DANCE CVAR_ENHANCEMENT("Cutscenes.SkipJiggyDance")

namespace {

constexpr f32 kDegToRad = (f32)M_PI / 180.0f;
constexpr f32 kLifetimeSec = 6.0f;

// Per-frame radius decay
f32 decayForBucket(int s2) {
    if (s2 < 2)
        return 0.992414f; // 0x7c
    if (s2 == 2)
        return 0.984886f; // 0x8c
    if (s2 < 5)
        return 0.97f; // 0x90
    if (s2 < 7)
        return 0.955339f; // 0x14
    if (s2 < 9)
        return 0.9409f; // 0x44
    if (s2 < 11)
        return 0.926679f; // 0x18
    if (s2 < 13)
        return 0.912673f; // 0xbc
    return 0.9f;          // 0xb8
}

// Rotate helper
void rotateVec(const f32 in[3], f32 aXDeg, f32 aYDeg, f32 out[3]) {
    f32 s1 = std::sin(aXDeg * kDegToRad), c1 = std::cos(aXDeg * kDegToRad);
    f32 s2 = std::sin(aYDeg * kDegToRad), c2 = std::cos(aYDeg * kDegToRad);
    f32 ix = in[0], iy = in[1], iz = in[2];
    f32 w = iz * c1 + iy * s1;
    out[1] = iy * c1 - iz * s1;
    out[0] = ix * c2 + w * s2;
    out[2] = w * c2 - ix * s2;
}

f32 vecLength(const f32 v[3]) {
    return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vecScale(f32 v[3], f32 s) {
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}

void vecSetLength(f32 v[3], f32 len) {
    f32 cur = vecLength(v);
    if (cur > 0.0001f) {
        vecScale(v, len / cur);
    }
}

int clamp255(int cur, int delta) {
    int sum = cur + delta;
    if (sum < 1) {
        return 0;
    }
    return sum < 255 ? sum : 255;
}

// Orbit state
void* sMarker = nullptr;
int sPhase;
f32 sOffset[3]; // orbit offset from Banjo (rotated each frame)
f32 sAngle;     // self-rotation accumulator
f32 sV94;       // phase-specific accumulator
f32 sV98;       // vertical climb
f32 sV9c;       // spin/wobble accumulator
f32 sVa8;       // phase-3 growth term
f32 sV128;      // pop-up term (rise = (1-v128)*40)
int sAlpha;     // 0..255
int sFrame;     // frame counter (chime + trail cadence)
f32 sElapsed;   // safety lifetime timer

void clearOrbit() {
    if (sMarker != nullptr) {
        marker_despawn((ActorMarker*)sMarker);
        sMarker = nullptr;
    }
}

void forgetOrbit() {
    sMarker = nullptr;
}

void emitSparkle(f32 x, f32 y, f32 z) {
    s16 sp[3] = { (s16)x, (s16)y, (s16)z };
    ParticleEmitter* e = __fxSparkle_create(sp, 0.0f, (enum asset_e)0x70F);
    if (e != nullptr) {
        particleEmitter_emitN(e, 1);
    }
}

extern "C" void spawnOrbit() {
    clearOrbit();

    f32 banjo[3];
    player_getPosition(banjo);

    sPhase = 0;
    sAngle = player_getYaw(); // 0x50 = raw yaw
    sV94 = sV98 = sV9c = sVa8 = sV128 = 0.0f;
    sAlpha = 0;
    sFrame = 0;
    sElapsed = 0.0f;
    const f32 unit[3] = { 0.0f, 0.0f, 1.0f };
    rotateVec(unit, 0.0f, sAngle + 180.0f, sOffset); // length 1

    f32 start[3] = { banjo[0] + sOffset[0], banjo[1] + sOffset[1], banjo[2] + sOffset[2] };
    s16 startS16[3] = { (s16)start[0], (s16)start[1], (s16)start[2] };
    Actor* actor = actor_spawnWithYaw_s16(ACTOR_46_JIGGY, &startS16, 0);
    if (actor == nullptr) {
        return;
    }
    actor->state = 0;
    actor_collisionOff(actor);
    actor->unk44_2 = 1; // Keep this jiggy out of jiggylist_map_actors
    actor->unk10_1 = 0; // Keep this jiggy out of map savestate
    sfxsource_playHighPriority(SFX_3E9_UNKNOWN);
    sMarker = (void*)actor->marker;
}

void updateOrbit() {
    if (sMarker == nullptr) {
        return;
    }
    f32 dt = time_getDelta();
    sElapsed += dt;
    sFrame++;
    if (sElapsed >= kLifetimeSec || sPhase >= 5) {
        clearOrbit();
        return;
    }
    Actor* actor = marker_getActor((ActorMarker*)sMarker);
    if (actor == nullptr) {
        sMarker = nullptr;
        return;
    }

    const f32 f26 = dt * 30.0f;
    const f32 f20 = f26 * 18.2f;
    bool spawnTrail = true;
    bool burst = false;

    switch (sPhase) {
        case 0: {
            f32 f24 = f26 * 15.0f;
            f32 len = vecLength(sOffset);
            int s2 = (int)(f26 * 4.0f);
            f32 decay = decayForBucket(s2);
            vecSetLength(sOffset, f24 + len);
            vecScale(sOffset, decay);
            sAlpha = clamp255(sAlpha, (int)(f26 * 16.0f));
            rotateVec(sOffset, 0.0f, f20, sOffset);
            spawnTrail = (sAlpha >= 0x50);
            sV98 = f26 * 0.3f;
            if (len >= 90.0f) {
                sPhase = 1;
            }
            break;
        }
        case 1: {
            rotateVec(sOffset, 0.0f, f20, sOffset);
            sAlpha = 0xff;
            sV98 += f26 * 3.5f;
            if (sV98 > 200.0f) {
                sPhase = 2;
                sV94 = 3.5f;
            }
            break;
        }
        case 2: {
            f32 len = vecLength(sOffset);
            f32 newLen = len + f26 * -3.0f;
            if (newLen > 0.0f) {
                vecSetLength(sOffset, newLen);
            }
            rotateVec(sOffset, 0.0f, f20, sOffset);
            sV9c += f26 * 0.03f;
            sV94 += f26 * -0.105f;
            if (len < f26 * 6.0f) {
                sPhase = 3;
                sV94 = 0.0f;
            } else if (sV94 > 0.0f) {
                sV98 += sV94;
            }
            break;
        }
        case 3: {
            sV94 += dt;
            sV9c += 2.0f * sV94 * f26;
            if (sV94 < 1.05f) {
                sVa8 = 0.25f;
            } else if (sV128 <= 0.2f) {
                burst = true;
                sPhase = 4;
                sV94 = 0.0f;
            } else {
                sVa8 += f26 * sVa8;
                sV128 -= f26 * sVa8;
                if (sV128 <= 0.0f) {
                    sV128 = 0.0f;
                    burst = true;
                    sPhase = 4;
                    sV94 = 0.0f;
                }
            }
            break;
        }
        case 4:
        default:
            clearOrbit();
            return;
    }

    sAngle += f20 + sV9c;
    sAngle = std::fmod(sAngle, 360.0f);
    if (sAngle < 0.0f) {
        sAngle += 360.0f;
    }

    f32 banjo[3];
    player_getPosition(banjo);
    f32 rise = (sV128 < 1.0f) ? (1.0f - sV128) * 80.0f * 0.5f : 0.0f;
    actor->position[0] = banjo[0] + sOffset[0];
    actor->position[1] = banjo[1] + sV98 + rise + sOffset[1];
    actor->position[2] = banjo[2] + sOffset[2];
    actor->yaw = sAngle;
    actor->yaw_ideal = sAngle;

    if (burst) {
        sfxsource_playHighPriority(SFX_3E9_UNKNOWN);
        sfxsource_playHighPriority((enum sfx_e)0x108);
        sfxsource_playHighPriority((enum sfx_e)0x30);
        for (int i = 0; i < 3; i++) {
            emitSparkle(actor->position[0], actor->position[1] - 10.0f, actor->position[2]);
        }
    }
    if (spawnTrail) {
        emitSparkle(actor->position[0], actor->position[1], actor->position[2]);
    }
}

} // namespace

// Romhacks that bundle wedarobi's jiggy dance should have it on by default.
// The hack may depend on the retained player motion that the animation provides.
static bool sJiggyDanceForced = false;

void RegisterJiggyCollect_Init() {
    clearOrbit();
    const bool skip = CVarGetInteger(CVAR_SKIP_JIGGY_DANCE, 0);
    const bool tooie = CVarGetInteger(CVAR_TOOIE_JIGGY_DANCE, 0) || sJiggyDanceForced;
    const bool playOrbit = (tooie && !skip) || IS_RANDO;

    COND_VB_SHOULD(VB_PLAY_JIGGY_DANCE, EVENT_PRIORITY_NORMAL, skip || tooie, { *should = false; });

    COND_HOOK(OnTooieJiggyCollect, EVENT_PRIORITY_NORMAL, playOrbit, [](IEvent*) { spawnOrbit(); });
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, playOrbit, [](IEvent*) { updateOrbit(); });
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, playOrbit, [](IEvent*) { forgetOrbit(); });
}

void TooieJiggyDance_ForceEnable() {
    sJiggyDanceForced = true;
    RegisterJiggyCollect_Init();
}

static RegisterShipInitFunc initJiggyCollect(RegisterJiggyCollect_Init,
                                             { CVAR_TOOIE_JIGGY_DANCE, CVAR_SKIP_JIGGY_DANCE, "IS_RANDO" });
