// This file should eventually go to LUS as the AI api.
//
// The audio interface, backed by the libultraship audio player. The decomp's
// audio manager runs its original path and this is the DAC at the end of it.

#include "OS.h"

#include <libultraship/libultraship.h>

#include "port/UI/cvar_prefixes.h"

extern "C" {
#include "port/Patches/Patches.h"
#include "libultraship/libultra/types.h"
#include "libultraship/libultra/os.h"
int32_t AudioPlayerBuffered(void);
int32_t AudioPlayerGetDesiredBuffered(void);
void AudioPlayerPlayFrame(const uint8_t* buf, size_t len);
}

#include <cstring>
#include <vector>

extern "C" u32 osAiGetLength(void) {
    int32_t excess = AudioPlayerBuffered() - AudioPlayerGetDesiredBuffered();
    return excess > 0 ? (u32)excess * 4 : 0;
}

// Audio pacing
namespace {

int32_t sCatchupOwed = 0;
bool sLastFrameWasCatchup = false;

int32_t AudioCatchupFrames() {
    constexpr int32_t kFrameSamples = 736;
    if (sLastFrameWasCatchup) {
        return 0;
    }
    if (port_audioHeld() || port_audioStallHold()) {
        return 0;
    }
    const int32_t deficit = AudioPlayerGetDesiredBuffered() - AudioPlayerBuffered();
    return deficit >= kFrameSamples ? 1 : 0;
}

} // namespace

extern "C" int32_t port_audioPumpShouldWait(void) {
    if (sCatchupOwed > 0) {
        sCatchupOwed--;
        sLastFrameWasCatchup = true;
        return 0;
    }
    sLastFrameWasCatchup = false;
    return 1;
}

extern "C" s32 osAiSetNextBuffer(void* buff, size_t len) {
    static bool sPrimed = false;
    if (!sPrimed) {
        sPrimed = true;
        std::vector<uint8_t> silence((size_t)AudioPlayerGetDesiredBuffered() * 4, 0);
        AudioPlayerPlayFrame(silence.data(), silence.size());
    }

    float masterVol = CVarGetInteger(CVAR_SETTING("Volume.Master"), 40) / 100.0f;
    if (masterVol >= 1.0f) {
        AudioPlayerPlayFrame((const uint8_t*)buff, len);
        sCatchupOwed = AudioCatchupFrames();
        return 0;
    }
    static thread_local std::vector<int16_t> scaled;
    scaled.resize(len / 2);
    const int16_t* src = (const int16_t*)buff;
    for (size_t i = 0; i < scaled.size(); i++) {
        scaled[i] = (int16_t)(src[i] * masterVol);
    }
    AudioPlayerPlayFrame((const uint8_t*)scaled.data(), len);
    sCatchupOwed = AudioCatchupFrames();
    return 0;
}
