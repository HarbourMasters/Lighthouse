#include "StaticData.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/ShipUtils.h"
#include "port/Rando/Rando.h"
// #include "port/Rando/Logic/Logic.h"

#include "enums.h"

namespace Rando {

namespace StaticData {

#define RI(id, article, name, type, actorId)      \
    {                                             \
        id, {                                     \
            id, #id, article, name, type, actorId \
        }                                         \
    }

// clang-format off
std::map<RandoItemId, RandoStaticItem> Items = {
    RI(RI_UNKNOWN,          "",     "Unknown",          RITYPE_UNKNOWN,         ACTOR_1_UNKNOWN),
    RI(RI_EMPTY_HONEYCOMB,  "an",   "Empty Honeycomb",  RITYPE_EMPTY_HONEYCOMB, ACTOR_47_EMPTY_HONEYCOMB),
    RI(RI_JIGGY,            "a",    "Jiggy",            RITYPE_JIGGY,           ACTOR_46_JIGGY),
    RI(RI_JINJO_BLUE,       "a",    "Blue Jinjo",       RITYPE_JINJO,           ACTOR_60_JINJO_BLUE),
    RI(RI_JINJO_GREEN,      "a",    "Green Jinjo",      RITYPE_JINJO,           ACTOR_62_JINJO_GREEN),
    RI(RI_JINJO_ORANGE,     "an",   "Orange Jinjo",     RITYPE_JINJO,           ACTOR_5F_JINJO_ORANGE),
    RI(RI_JINJO_PINK,       "a",    "Pink Jinjo",       RITYPE_JINJO,           ACTOR_61_JINJO_PINK),
    RI(RI_JINJO_YELLOW,     "a",    "Yellow Jinjo",     RITYPE_JINJO,           ACTOR_5E_JINJO_YELLOW),
    RI(RI_MOLEHILL,         "a",    "New Ability",      RITYPE_MOLEHILL,        ACTOR_12C_MOLEHILL),
    RI(RI_MUMBO_TOKEN,      "a",    "Mumbo Token",      RITYPE_MUMBO_TOKEN,     ACTOR_2D_MUMBO_TOKEN),
    RI(RI_MUSIC_NOTE,       "a",    "Note",             RITYPE_MUSIC_NOTE,      ACTOR_51_MUSIC_NOTE),
};
// clang-format on

RandoItemId GetRandoItemByActorId(actor_e actorId) {
    for (auto& [randoItemId, randoStaticItem] : Items) {
        if (randoStaticItem.actorId == actorId) {
            return randoItemId;
        }
    }
    return RI_UNKNOWN;
}

} // namespace StaticData
} // namespace Rando