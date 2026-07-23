// Dev tool: simulates a teammate's remote note/jinjo collects locally. Toggled by Dev Tools >
// General "Simulate Remote Collects".
//
// libultraship first: core2/timedfunc.h redefines `reinterpret_cast`, breaking MSVC's stdlib.
#include <libultraship/libultraship.h>
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/UI/cvar_prefixes.h"
#include "port/UI/Notification.h"

#include <chrono>
#include <string>

extern "C" {
#include "enums.h"
#include "functions.h"
}

extern "C" void port_noteRetention_applyRemoteCollect(int32_t mapId, int32_t noteIndex, int32_t sameMap);
extern "C" int32_t port_noteRetention_debugPickLive(int32_t mapId);
extern "C" void port_jinjoRetention_applyRemoteCollect(int32_t map, int32_t bit, int32_t sameMap);
extern "C" int32_t port_jinjoRetention_debugPickLive(void);

#define CVAR_SIM_REMOTE_COLLECT CVAR_DEVELOPER_TOOLS("SimRemoteCollect")

namespace {

using Clock = std::chrono::steady_clock;
constexpr std::chrono::seconds kInterval{ 30 };

bool sArmed = false;
int32_t sLastMap = -1;
Clock::time_point sNextFire;
bool sPreferNote = true; // alternate note/jinjo across successful ticks

const char* jinjoColorName(int32_t bit) {
    switch (bit) {
        case 1 << 0: return "blue";
        case 1 << 1: return "green";
        case 1 << 2: return "orange";
        case 1 << 3: return "pink";
        case 1 << 4: return "yellow";
        default: return "?";
    }
}

bool SimulateOne(bool note, int32_t map) {
    if (note) {
        int32_t idx = port_noteRetention_debugPickLive(map);
        if (idx < 0) {
            return false;
        }
        port_noteRetention_applyRemoteCollect(map, idx, 1);
        Notification::Emit({ .message = "[Sim] remote note collect #" + std::to_string(idx) });
        return true;
    }
    int32_t bit = port_jinjoRetention_debugPickLive();
    if (bit == 0) {
        return false;
    }
    port_jinjoRetention_applyRemoteCollect(map, bit, 1);
    Notification::Emit({ .message = std::string("[Sim] remote jinjo collect (") + jinjoColorName(bit) + ")" });
    return true;
}

void Tick() {
    if (!retention::systemActive()) {
        sArmed = false;
        return;
    }
    int32_t map = (int32_t)gsworld_getMap();
    Clock::time_point now = Clock::now();
    if (!sArmed || map != sLastMap) {
        sArmed = true;
        sLastMap = map;
        sNextFire = now + kInterval;
        return;
    }
    if (now < sNextFire) {
        return;
    }
    sNextFire = now + kInterval;
    if (SimulateOne(sPreferNote, map)) {
        sPreferNote = !sPreferNote;
    } else {
        SimulateOne(!sPreferNote, map);
    }
}

void RegisterRemoteCollectSim_Init() {
    sArmed = false;
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SIM_REMOTE_COLLECT, 0),
              [](IEvent*) { Tick(); });
}

RegisterShipInitFunc initRemoteCollectSim(RegisterRemoteCollectSim_Init, { CVAR_SIM_REMOTE_COLLECT });

} // namespace
