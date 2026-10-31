#include "GeoCull.h"

#include "port/Enhancements/Events/Hooks/Events.h"

extern "C" {
#include "core2/model.h"
}

static int sConsumerMask = 0;

static void ForwardToConsumers(bool* should, va_list args) {
    int type = va_arg(args, int);
    const void* cmd = va_arg(args, const void*);
    const void* modelBin = va_arg(args, const void*);
    const unsigned char* areaIds = va_arg(args, const unsigned char*);
    int areaCount = va_arg(args, int);
    int detail0 = va_arg(args, int);
    int detail1 = va_arg(args, int);

    // Key commands by their offset within the geo command list.
    const auto* bin = static_cast<const BKModelBin*>(modelBin);
    const unsigned char* geoBase = (const unsigned char*)modelBin;
    if (bin != nullptr) {
        geoBase += bin->geo_list_offset;
    }
    int offset = (int)((const unsigned char*)cmd - geoBase);
    bool forceDraw = false;
    CALL_EVENT(OnGeoCull, type, offset, modelBin, areaIds, areaCount, detail0, detail1, *should, &forceDraw);
    // An UnkE with no branch would run itself again.
    *should = *should ||
              (forceDraw && (type != OCCLUSION_CMD_UNKE || static_cast<const geo_cmd_E_s*>(cmd)->branch_offset != 0));
}

extern "C" void GeoCull_SetConsumer(int consumerBit, int active) {
    if (active) {
        sConsumerMask |= consumerBit;
    } else {
        sConsumerMask &= ~consumerBit;
    }
    COND_VB_SHOULD(VB_DRAW_GEO_CMD, EVENT_PRIORITY_NORMAL, sConsumerMask != 0, { ForwardToConsumers(should, args); });
}
