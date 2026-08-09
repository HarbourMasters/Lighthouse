#include "GeoCull.h"

#include "port/Enhancements/Events/Hooks/Events.h"

extern "C" {
#include "model.h"
}

static int sConsumerMask = 0;

extern "C" void GeoCull_SetConsumer(int consumerBit, int active) {
    if (active) {
        sConsumerMask |= consumerBit;
    } else {
        sConsumerMask &= ~consumerBit;
    }
}

extern "C" int port_geoCullDraw(int type, const void* cmd, const void* modelBin, int drawnVanilla,
                                const unsigned char* areaIds, int areaCount, int detail0, int detail1) {
    // Nothing listening: leave the vanilla decision untouched and skip the event entirely
    // (CallEvent does a string-keyed map lookup per call, which we don't want per geo command
    // in normal play).
    if (sConsumerMask == 0) {
        return drawnVanilla;
    }

    // Key commands by their offset within the geo command list.
    const auto* bin = static_cast<const BKModelBin*>(modelBin);
    const unsigned char* geoBase = (const unsigned char*)modelBin;
    if (bin != nullptr) {
        geoBase += bin->geo_list_offset;
    }
    int offset = (int)((const unsigned char*)cmd - geoBase);
    bool forceDraw = false;
    CALL_EVENT(OnGeoCull, type, offset, modelBin, areaIds, areaCount, detail0, detail1, drawnVanilla, &forceDraw);
    return (drawnVanilla || forceDraw) ? 1 : 0;
}
