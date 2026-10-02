#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/Shared/HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "core2/model.h"
}

// The Tooie jiggy animation exists in this hack
void TooieJiggyDance_ForceEnable();

// The hack draws these from far away, so they don't pop in
constexpr SpawnRewrite kBubblingBogSpawnRewrites[] = {
    { ACTOR_F_CHIMPY, 0, 0x400, 0x8000 },
    { ACTOR_F1_LEAF_BOAT, 0, 0x400, 0x8000 },
    { ACTOR_340_XMAS_TREE_ICE, 0, 0x400, 0x8000 },
};

static bool BubblingBog_BogPainted(const Vtx& vertex, u8 blueMask) {
    return vertex.v.cn[0] == 0xFF && vertex.v.cn[1] == 0xFF && (vertex.v.cn[2] & blueMask) == 0x04;
}

// The hack flags every triangle whose vertices are all painted FF FF 04, then whitens that paint.
static void BubblingBog_RestoreBogHazard(BKModelBin* bin) {
    BKCollisionList* collisionList = bin != nullptr ? modelbin_getCollisionList(bin) : nullptr;
    if (collisionList == nullptr) {
        return;
    }
    Vtx* vertices = modelbin_getVtxList(bin)->vertices;
    BKCollisionTriangle* triangles =
        (BKCollisionTriangle*)(collisionList->data + collisionList->geo_count * sizeof(BKCollisionGeometry));

    for (s32 i = 0; i < collisionList->tri_count; i++) {
        const u16* corners = (const u16*)triangles[i].unk0;
        if (BubblingBog_BogPainted(vertices[corners[0]], 0xFF) && BubblingBog_BogPainted(vertices[corners[1]], 0xFF) &&
            BubblingBog_BogPainted(vertices[corners[2]], 0xFF)) {
            triangles[i].flags = 0x2000;
        }
    }
    for (s32 i = 0; i < collisionList->tri_count; i++) {
        const u16* corners = (const u16*)triangles[i].unk0;
        for (s32 corner = 0; corner < 3; corner++) {
            if (BubblingBog_BogPainted(vertices[corners[corner]], 0xFE)) {
                vertices[corners[corner]].v.cn[2] = 0xFF;
            }
        }
    }
}

void RegisterBubblingBogPatches() {
    TooieJiggyDance_ForceEnable();
    HackShared_EnableForceAbilitiesUsed(kAllUsedAbilities);
    HackShared_EnableSpawnRewrites(kBubblingBogSpawnRewrites);

    REGISTER_LISTENER(OnMapLoadStub, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        if (gsworld_getMap() == MAP_D_BGS_BUBBLEGLOOP_SWAMP) {
            BubblingBog_RestoreBogHazard(mapModel_getModelBin(0));
        }
    });
}
