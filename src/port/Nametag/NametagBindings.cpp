// BK-specific bindings for the portable Nametag overlay: projection, distance,
// FB dimensions, and the per-draw tick listener.

#include <libultraship/libultraship.h>

#include "Nametag.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "core1/viewport.h"
extern int gFramebufferWidth;
extern int gFramebufferHeight;
}

namespace {

void InitNametagBindings() {
    Nametag::SetProjectFn(&viewport_func_8024E030);
    Nametag::SetDistanceFn(&viewport_getDistance);
    Nametag::SetNativeFramebufferSize(&gFramebufferWidth, &gFramebufferHeight);
    Nametag::RegisterOverlay();
}

} // namespace

static RegisterShipInitFunc initNametagBindings(InitNametagBindings, { "BOOT" });

// Tags are queued during the world draw, so the queue turns over on OnPlayerDraw rather than
// on the game tick. Ordering against the producers on this event does not matter.
static RegisterShipInitFunc init([]() {
    REGISTER_LISTENER(OnPlayerDraw, EVENT_PRIORITY_HIGH, [](IEvent* event) { Nametag::BeginDraw(); });
});
