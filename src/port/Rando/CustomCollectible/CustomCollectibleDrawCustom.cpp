// Custom item model rendering.
//
// The models live in lighthouse.o2r as XML libultraship resources under
// objects/object_X_item.
// They are untextured prim-color geometry, so they draw with a single matrix
// push and no material setup beyond what the display lists carry.

// Include order matters here.
// 1. ResourceHelpers.h (via ship/Context.h -> spdlog) parses the Windows headers,
//    which must happen before any decomp header: Events.h drags in prop.h ->
//    structs.h -> bool.h, whose function-like BOOL(x) macro corrupts the
//    `typedef BOOL (CALLBACK* ...)` declarations in the Windows SDK.
// 2. bridge.h/Events.h bring in the events bridge/json templates, which must be
//    seen with C++ linkage before functions.h re-includes them inside the
//    extern "C" block below.
#include <libultraship/bridge.h>
#include "port/ResourceHelpers.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Interpolation/FrameInterpolation.h"

#include "CustomCollectibleDrawCustom.h"
#include "CustomCollectible.h"

extern "C" {
#include <ultra64.h>
#include "macros.h"
#include "prop.h"
#include "functions.h"
#include "variables.h"
#include "core1/viewport.h"
#include "core1/mlmtx.h"
}

static const char* sCustomModelPaths[] = {
    "objects/object_archipelago_item/gArchipelagoItemDL",
    "objects/object_archipelago_item/gArchipelagoJunkDL",
    "objects/object_archipelago_item/gArchipelagoProgressiveDL",
};

// The models are authored at OoT world scale, which is roughly 10x too large
// for BK. Baked into every draw so callers treat scale 1.0 as "natural size".
static const f32 sCustomModelBaseScale = 0.04f;

// BK never uploads RSP lights (its models bake shading into vertex colors), but
// these materials enable G_LIGHTING, so without our own light setup the shade
// input is garbage and the model renders black. Neutral white key light from
// the upper-front-right plus a mid ambient.
static Lights1 sCustomModelLights = gdSPDefLights1(110, 110, 110, 255, 255, 255, 40, 40, 40);

// Pin the resources so the cached Gfx pointers stay valid across frames.
static std::shared_ptr<Ship::IResource> sModelResources[ARRAY_COUNT(sCustomModelPaths)];
static Gfx* sModelDLs[ARRAY_COUNT(sCustomModelPaths)];

static Gfx* CustomCollectible_GetDL(CustomItemModel model) {
    if (model < 0 || model >= (s32)ARRAY_COUNT(sCustomModelPaths)) {
        return nullptr;
    }
    if (sModelDLs[model] == nullptr) {
        sModelResources[model] = GetResourceByName(sCustomModelPaths[model]);
        if (sModelResources[model] == nullptr) {
            return nullptr;
        }
        sModelDLs[model] = ResourceMgr_LoadGfxByName(sCustomModelPaths[model]);
    }
    return sModelDLs[model];
}

Actor* CustomCollectible_DrawCustomModel(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx) {
    Actor* actor = marker_getActor(marker);
    ActorLocal_CustomCollectible* customLocal = (ActorLocal_CustomCollectible*)&actor->local;
    CustomItemModel model;

    switch (customLocal->randoItemId) {
        case RI_AP_ITEM_PROGRESSION:
            model = ARCHIPELAGO_MODEL_PROGRESSIVE;
            break;
        default:
            break;
    }

    Gfx* dl = CustomCollectible_GetDL(model);
    if (dl == nullptr) {
        return actor;
    }

    f32 camera_position[3];
    viewport_getPosition_vec3f(camera_position);

    // The mlMtx calls below are recorded for frame interpolation; the scope key
    // pairs this draw's matrix across ticks. Keyed by model, so two instances
    // of the same model drawn in the same frame would need instance-keyed
    // scopes instead.
    FrameInterpolation_RecordOpenChild("custom_item", (uintptr_t)model);
    f32 rotation[3] = { actor->pitch, actor->yaw, actor->roll };
    f32 position[3] = { actor->position[0], actor->position[1] + 50, actor->position[2] };
    // The modelview translation is camera-relative, matching modelRender_draw.
    mlMtxIdent();
    func_80252AF0(camera_position, position, rotation, actor->scale * sCustomModelBaseScale, NULL);
    mlMtxApply(*mtx);
    gSPMatrix((*gfx)++, (*mtx)++, G_MTX_PUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPSetLights1((*gfx)++, sCustomModelLights);
    gSPDisplayList((*gfx)++, dl);
    gSPPopMatrix((*gfx)++, G_MTX_MODELVIEW);

    FrameInterpolation_RecordCloseChild();

    return actor;
}
