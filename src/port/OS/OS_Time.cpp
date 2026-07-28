// This file should eventually go back to LUS; it is unchanged from its
// libultra/os.cpp original, and only lives here because taking osSetTimer
// port-side means taking that whole object with it.

#include <chrono>
#include <ratio>

extern "C" {
#include "libultraship/libultra/types.h"
#include "libultraship/libultra/os.h"
}

// A chrono duration matching the N64's 46.875MHz clock rate.
typedef std::ratio<3000, 64> n64ClockRatio;
typedef std::ratio_divide<std::micro, n64ClockRatio> n64CycleRate;
typedef std::chrono::duration<long long, n64CycleRate> n64CycleRateDuration;

extern "C" {

uint64_t __osCurrentTime = 0;

void osSetTime(OSTime time) {
    __osCurrentTime =
        std::chrono::duration_cast<n64CycleRateDuration>(std::chrono::steady_clock::now().time_since_epoch()).count() +
        time;
}

uint64_t osGetTime() {
    return std::chrono::duration_cast<n64CycleRateDuration>(std::chrono::steady_clock::now().time_since_epoch())
               .count() -
           __osCurrentTime;
}

uint32_t osGetCount() {
    return std::chrono::duration_cast<n64CycleRateDuration>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

} // extern "C"
