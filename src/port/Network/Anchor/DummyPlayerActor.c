#include <ultra64.h>
#include "functions.h"
#include "variables.h"

static Actor* dummyPlayerAnchor_draw(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);
static void dummyPlayerAnchor_update(Actor* this);

ActorInfo gDummyPlayerAnchorInfo = { MARKER_29F_DUMMY_PLAYER_ANCHOR,
                                     ACTOR_3CC_DUMMY_PLAYER_ANCHOR,
                                     ASSET_0_NONE,
                                     0,
                                     NULL,
                                     dummyPlayerAnchor_update,
                                     actor_update_func_80326224,
                                     dummyPlayerAnchor_draw,
                                     0,
                                     0,
                                     0.0f,
                                     0 };

static void dummyPlayerAnchor_update(Actor* this) {
    if (!this->initialized) {
        this->initialized = true;
        this->marker->collidable = false;
    }
}

static Actor* dummyPlayerAnchor_draw(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx) {
    return marker_getActor(marker);
}
