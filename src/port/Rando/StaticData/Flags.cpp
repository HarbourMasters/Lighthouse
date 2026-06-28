#include "StaticData.h"

#include <cstring>

namespace Rando {

namespace StaticData {

#define RF(id, defaultValue)      \
    {                             \
        id, {                     \
            id, #id, defaultValue \
        }                         \
    }

// clang-format off
std::map<RandoInf, RandoStaticFlag> Flags = {
    RF(RANDO_INF_UNKNOWN,                           RO_GENERIC_OFF),
    RF(RANDO_INF_ANCHOR_RAISED,                     RO_GENERIC_OFF),
    RF(RANDO_INF_BRIDGE_REPAIRED_DIALOG_COMPLETE,   RO_GENERIC_OFF),
    RF(RANDO_INF_CLANKER_RAISED,                    RO_GENERIC_OFF),
    RF(RANDO_INF_MINIGAME_RINGS_COMPLETED,          RO_GENERIC_OFF),
    RF(RANDO_INF_WATER_PYRAMID_DRAINED,             RO_GENERIC_OFF),
};
// clang-format on

} // namespace StaticData

} // namespace Rando
