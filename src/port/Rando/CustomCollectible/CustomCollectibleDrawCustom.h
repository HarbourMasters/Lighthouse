#ifndef PORT_ARCHIPELAGO_ITEM_H
#define PORT_ARCHIPELAGO_ITEM_H

#include <libultra/gbi.h>
#include <libultraship/libultra/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations (tag structs from prop.h) so this header stays free of
// the decomp includes — including prop.h here would drag in bool.h, whose
// function-like BOOL() macro corrupts the Windows SDK headers in C++ TUs.
struct actorMarker_s;
struct actor_s;

typedef enum CustomItemModel {
    ARCHIPELAGO_MODEL_NONE = -1, // resolver sentinel: keep the vanilla model
    ARCHIPELAGO_MODEL_ITEM,
    ARCHIPELAGO_MODEL_JUNK,
    ARCHIPELAGO_MODEL_PROGRESSIVE,
} CustomItemModel;

// Draws an Archipelago item model at a world position. Must be called from a
// draw context that owns the display list and matrix pools (an actor draw
// callback, a world/player draw event hook, cutscene draw code, etc.).
// rotation is pitch/yaw/roll in degrees and may be NULL; scale is a relative
// multiplier where 1.0 = natural in-world size (the OoT-scale source models are
// shrunk to BK proportions internally).
Actor* CustomCollectible_DrawCustomModel(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);

#ifdef __cplusplus
}
#endif

#endif
