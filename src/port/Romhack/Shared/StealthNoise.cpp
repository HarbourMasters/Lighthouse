#include <libultraship/bridge.h>
#include <cmath>
#include <spdlog/spdlog.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "bk_time.h"
#include "core1/core1.h"
#include "core2/timedfunc.h"
#include "core1/pfsmanager.h"

extern f32 D_8037C5B0[3];
extern PfsManagerControllerData D_80281138[4];
extern Gfx D_80369238[];
f32 randf(void);
s32 getGameMode(void);

typedef struct {
    s16 x, y, topVertexAlpha, bottomVertexAlpha;
    u8 fmtString[8];
    f32 scale;
    u8* string;
    u8 rgba[4];
} StealthPrintBuffer;
extern StealthPrintBuffer* print_sCurrentPtr;
bool gcpausemenu_isCapturing(void);
}

namespace {

// Constants
constexpr f32 kHudLerp = 0.25f;
constexpr f32 kAlphaLerp = 0.2f;
constexpr f32 kAlphaEpsilon = 0.01f;
constexpr f32 kHudShownX = 20.0f;
constexpr f32 kHudHiddenX = -70.0f;
constexpr f32 kHudYDialog = 137.0f;
constexpr f32 kHudYPlain = 177.0f;
constexpr f32 kAlphaNear = 1.0f;
constexpr f32 kAlphaFar = 0.6f;
constexpr f32 kStickDeadzone = 0.1f;
constexpr f32 kGainScaleInner = 0.4f;
constexpr f32 kGainScaleOuter = 0.46f;
constexpr f32 kGainJitter = 0.05f;
constexpr f32 kDecay = 0.55f;
constexpr f32 kDecayFloor = 0.1f;
constexpr f32 kEggPeriodHead = 0.4f;
constexpr f32 kEggPeriodAss = 0.6f;
constexpr f32 kEggJitter = 0.07f;
constexpr f32 kLoudAirborne = 0.85f;
constexpr f32 kLoudAirJitter = 0.1f;
constexpr f32 kCatchCamDelay = 0.72f;
constexpr f32 kLabelScale = 0.75f;
constexpr s32 kAirborneForLanding = 3;
constexpr u32 kCatchSfxPacked = 0xB31FF86B;
constexpr s32 kCatchSfxA = 0x142;
constexpr s32 kCatchSfxB = 0x14B;

// HUD geometry
constexpr s16 kLabelOffX = 8, kLabelOffY = 8;
constexpr s16 kFrameX = 0, kFrameY = 0, kFrameW = 85, kFrameH = 20;
constexpr s16 kBarX = 4, kBarY = 12, kBarW = 77, kBarH = 5;
constexpr f32 kFrameAlphaScale = 160.0f;
constexpr f32 kBarAlphaScale = 136.0f;

// Quiet-states table
constexpr s32 kQuietStates[] = {
    BS_0_NONE,
    BS_1_IDLE,
    BS_2_WALK_SLOW,
    BS_1F_WALK_CREEP,
    BS_5A_LOADZONE,
    BS_7_CROUCH,
    BS_7A_WALK_MUD,
    BS_98_WALK_DRONE,
    BS_9_EGG_HEAD,
    BS_A_EGG_ASS,
    BS_14_BTROT_ENTER,
    BS_15_BTROT_IDLE,
    BS_17_BTROT_EXIT,
    BS_1A_WONDERWING_ENTER,
    BS_1B_WONDERWING_IDLE,
    BS_1E_WONDERWING_EXIT,
};

struct NoiseState {
    s32 airborneFrames;
    bool loudLanding;
    f32 noise;
    f32 gain;
    f32 burstTimer;
    f32 burstAmount;
    f32 eggTimer;
    bool hudRetracted;
    f32 hudX, hudY, hudAlpha;
    f32 targetX, targetY, targetAlpha;
    bool hudOffscreen;
};

NoiseState sState{};
const StealthNoiseConfig* sCfg = nullptr;
bool sCaughtBefore = false;
bool sSuspended = false;
bool sCatchPending = false;

f32 Clamp01(f32 value) {
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

void Approach(f32& current, f32 target, f32 rate, f32 epsilon) {
    const f32 delta = target - current;
    if (fabsf(delta) > epsilon) {
        current += delta * rate;
    }
}

bool IsQuiet(s32 state) {
    for (s32 quietState : kQuietStates) {
        if (quietState == state) {
            return true;
        }
    }
    return false;
}

void ResetMeter() {
    sState.airborneFrames = 0;
    sState.loudLanding = false;
    sState.noise = 0.0f;
    sState.eggTimer = 0.0f;
    sState.burstTimer = 0.0f;
    sState.targetX = kHudHiddenX;
    sState.targetY = kHudYPlain;
    sState.targetAlpha = kAlphaFar;
    sState.hudOffscreen = true;
}

void RetractHud() {
    sState.hudRetracted = true;
}

void TriggerCatch() {
    sState.noise = 1.0f;
    func_8030E624(kCatchSfxPacked);
    gcdialog_showDialog(sCfg->caughtTextId, sCaughtBefore ? 0x02 : 0x82, NULL, NULL, NULL, NULL);
    func_80324E38(0.0f, 1);
    timed_setStaticCameraToNode(kCatchCamDelay, 1);
    timedFunc_set_0(kCatchCamDelay, RetractHud);
    sCaughtBefore = true;
    sCatchPending = true;
}

void ResolveCatch() {
    gcsfx_play((enum sfx_e)(sfx_randi2(0, 2) != 0 ? kCatchSfxA : kCatchSfxB));
    if (!mumboscore_get((enum mumbotoken_e)sCfg->firstCatchToken)) {
        const s32 warpDestination = port_getRomhackWarpDest(sCfg->escapeWarpIndex);
        if (warpDestination >= 0) {
            func_8031CC8C(NULL, warpDestination);
        } else if (sCfg->escapeWarpFallback != nullptr) {
            sCfg->escapeWarpFallback(NULL, NULL);
        }
        mumboscore_set((enum mumbotoken_e)sCfg->firstCatchToken, 1);
    } else {
        transitionToMap((enum map_e)gsworld_getMap(), gsworld_getExit(), 1);
    }
    sSuspended = true;
    sCatchPending = false;
}

void Update() {
    if (sCfg == nullptr || gsworld_getMap() != sCfg->map) {
        return;
    }
    if (game_is_frozen() || gcpausemenu_isCapturing()) {
        return;
    }
    NoiseState& meter = sState;

    Approach(meter.hudX, meter.targetX, kHudLerp, 1.0f);
    Approach(meter.hudY, meter.targetY, kHudLerp, 1.0f);
    Approach(meter.hudAlpha, meter.targetAlpha, kAlphaLerp, kAlphaEpsilon);
    meter.hudAlpha = Clamp01(meter.hudAlpha);

    if (meter.burstTimer > 0.0f) {
        meter.burstTimer -= time_getDelta();
        if (meter.burstTimer < 0.0f) {
            meter.burstTimer = 0.0f;
        }
    } else {
        meter.burstAmount = 0.0f;
    }

    const bool dialogUp = gcdialog_hasCurrentTextId() != 0;
    if (!dialogUp && sCatchPending) {
        ResolveCatch();
    }

    const f32 playerX = D_8037C5B0[0];
    const bool inRoom = playerX >= sCfg->zoneMin && playerX <= sCfg->zoneMax;
    if (inRoom && !meter.hudRetracted) {
        meter.hudOffscreen = false;
    } else if (!inRoom) {
        meter.hudOffscreen = true;
    }

    print_dialog((s32)(kLabelOffX + meter.hudX), (s32)(kLabelOffY + meter.hudY), (u8*)"NOISE");
    if (print_sCurrentPtr != nullptr) {
        print_sCurrentPtr->scale = kLabelScale;
        if (inRoom && !meter.hudOffscreen) {
            meter.targetAlpha = (playerX >= sCfg->innerMin && playerX <= sCfg->innerMax) ? kAlphaNear : kAlphaFar;
        } else {
            meter.targetAlpha = 0.0f;
        }
        print_sCurrentPtr->rgba[0] = 0xFF;
        print_sCurrentPtr->rgba[1] = 0xFF;
        print_sCurrentPtr->rgba[2] = 0xFF;
        print_sCurrentPtr->rgba[3] = (u8)(meter.hudAlpha * 255.0f);
    }

    meter.targetY = dialogUp ? kHudYDialog : kHudYPlain;
    meter.targetX = meter.hudRetracted ? kHudHiddenX : kHudShownX;
    meter.hudOffscreen = meter.hudRetracted;

    if (sSuspended || sCatchPending) {
        return;
    }

    const s32 state = bs_getState();
    const bool quiet = IsQuiet(state);
    const bool stable = player_isStable() != 0;

    const bool inner = playerX >= sCfg->innerMin && playerX <= sCfg->innerMax;
    if (!inRoom) {
        ResetMeter();
        meter.loudLanding = false;
        if (!quiet) {
            meter.noise = stable ? 1.0f : kLoudAirborne + randf() * kLoudAirJitter;
        }
        meter.noise = Clamp01(meter.noise);
        return;
    }

    const bool muted = !stable || D_80281138[0].side_button2.button_z != 0 || state == BS_73_UNKNOWN ||
                       state == BS_79_BTROT_LOCKED || state == BS_98_WALK_DRONE || state == BS_1E_WONDERWING_EXIT ||
                       (state & ~0x10) == BS_7_CROUCH;
    f32 gain = 0.0f;
    if (!muted) {
        gain = Clamp01((bastick_distance() - kStickDeadzone) / (inner ? kGainScaleInner : kGainScaleOuter));
    }
    meter.gain = Clamp01(gain + randf() * kGainJitter);

    if (state == BS_9_EGG_HEAD || state == BS_A_EGG_ASS) {
        meter.eggTimer += time_getDelta();
        const f32 period = (state == BS_9_EGG_HEAD) ? kEggPeriodHead : kEggPeriodAss;
        meter.noise = (meter.eggTimer > period ? kStickDeadzone : 0.0f) + meter.burstAmount + randf() * kEggJitter;
    } else {
        meter.noise = (inner && stable) ? meter.gain : meter.noise + meter.gain;
        meter.eggTimer = 0.0f;
        if (!stable) {
            meter.airborneFrames++;
            if (!inner) {
                meter.loudLanding = false;
            }
        } else {
            if (meter.airborneFrames >= kAirborneForLanding) {
                meter.loudLanding = true;
                meter.noise = 1.0f;
            }
            if (!inner) {
                meter.loudLanding = false;
                const f32 decayed = meter.noise * kDecay;
                meter.airborneFrames = 0;
                meter.noise = (decayed < kDecayFloor) ? 0.0f : decayed;
                meter.noise += randf() * kGainJitter;
            }
        }
    }

    if (state != BS_73_UNKNOWN && state != BS_79_BTROT_LOCKED && !quiet) {
        meter.noise = stable ? 1.0f : kLoudAirborne + randf() * kLoudAirJitter;
    }
    meter.noise = Clamp01(meter.noise);

    if (inner && stable && (!quiet || meter.loudLanding)) {
        TriggerCatch();
    }
}

void FillRect(Gfx** gfx, s32 left, s32 top, s32 width, s32 height, u8 red, u8 green, u8 blue, u8 alpha) {
    if (width <= 0 || height <= 0 || alpha == 0) {
        return;
    }
    gDPPipeSync((*gfx)++);
    gDPSetPrimColor((*gfx)++, 0, 0, red, green, blue, alpha);
    gDPSetCombineMode((*gfx)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPScisFillRectangle((*gfx)++, left, top, left + width - 1, top + height - 1);
    gDPPipeSync((*gfx)++);
}

void BarColour(s32 percent, u8* red, u8* green) {
    const s32 remaining = 100 - percent;
    if (percent >= 100) {
        *red = 0xFF;
        *green = 0x00;
    } else if (remaining >= 100) {
        *red = 0x00;
        *green = 0xFF;
    } else if (remaining < 40) {
        *red = 0xFF;
        *green = (u8)((255 * remaining) / 40);
    } else {
        *red = (u8)((255 * percent) / 60);
        *green = 0xFF;
    }
}

void Draw(Gfx** gfx) {
    if (sCfg == nullptr || gsworld_getMap() != sCfg->map || getGameMode() == GAME_MODE_4_PAUSED) {
        return;
    }
    const NoiseState& meter = sState;
    const u8 frameAlpha = (u8)(meter.hudAlpha * kFrameAlphaScale);
    if (frameAlpha == 0) {
        return;
    }

    gSPDisplayList((*gfx)++, D_80369238);
    FillRect(gfx, (s32)(kFrameX + meter.hudX), (s32)(kFrameY + meter.hudY), kFrameW, kFrameH, 0x00, 0x00, 0x00,
             frameAlpha);

    s32 percent = (s32)(meter.noise * 100.0f);
    percent = percent < 0 ? 0 : (percent > 100 ? 100 : percent);
    u8 red, green;
    BarColour(percent, &red, &green);
    FillRect(gfx, (s32)(kBarX + meter.hudX), (s32)(kBarY + meter.hudY), (s32)(kBarW * meter.noise), kBarH, red, green,
             0x00, (u8)(meter.hudAlpha * kBarAlphaScale));
}

} // namespace

void StealthNoise_AddBurst(float amount, float seconds) {
    sState.burstAmount = amount;
    sState.burstTimer = seconds;
}

void StealthNoise_Enable(const StealthNoiseConfig& cfg) {
    sCfg = &cfg;
    ResetMeter();

    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) { Update(); });

    REGISTER_LISTENER(OnHudDraw, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* hudEvent = reinterpret_cast<OnHudDraw*>(event);
        Draw(hudEvent->gfx);
    });

    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        sSuspended = false;
        sCatchPending = false;
        sState.hudRetracted = false;
        ResetMeter();
    });
}
