// This file should eventually go to LUS as the AI api.
//
// The audio interface, backed by the libultraship audio player. The decomp's
// audio manager runs its original path and this is the DAC at the end of it.

#include "OS.h"

#include <libultraship/libultraship.h>

#include "port/UI/cvar_prefixes.h"

extern "C" {
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

extern "C" s32 osAiSetNextBuffer(void* buff, size_t len) {
    static bool sPrimed = false;
    if (!sPrimed) {
        sPrimed = true;
        std::vector<uint8_t> silence((size_t)AudioPlayerGetDesiredBuffered() * 4, 0);
        AudioPlayerPlayFrame(silence.data(), silence.size());
    }

    float masterVol = CVarGetInteger(CVAR_SETTING("Volume.Master"), 100) / 100.0f;
    if (masterVol >= 1.0f) {
        AudioPlayerPlayFrame((const uint8_t*)buff, len);
        return 0;
    }
    std::vector<int16_t> scaled(len / 2);
    const int16_t* src = (const int16_t*)buff;
    for (size_t i = 0; i < scaled.size(); i++) {
        scaled[i] = (int16_t)(src[i] * masterVol);
    }
    AudioPlayerPlayFrame((const uint8_t*)scaled.data(), len);
    return 0;
}
