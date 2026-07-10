#include "StaticData.h"

#include <cstring>

namespace Rando {

namespace StaticData {

#define RO(id, defaultValue)                             \
    {                                                    \
        id, {                                            \
            id, #id, "gRando.Options." #id, defaultValue \
        }                                                \
    }

// clang-format off
std::map<RandoOptionId, RandoStaticOption> Options = {
    RO(RO_LOGIC,                        RO_LOGIC_GLITCHLESS),
    RO(RO_SPAWN_JUNK,                   RO_GENERIC_OFF),
    RO(RO_SHUFFLE_BEEHIVE_HONEYCOMBS,   RO_GENERIC_OFF),
    RO(RO_SHUFFLE_BLUE_EGGS,            RO_GENERIC_OFF),
    RO(RO_SHUFFLE_EMPTY_HONEYCOMBS,     RO_GENERIC_OFF),
    RO(RO_SHUFFLE_EXTRA_LIVES,          RO_GENERIC_OFF),
    RO(RO_SHUFFLE_JIGGIES,              RO_GENERIC_OFF),
    RO(RO_SHUFFLE_JINJOS,               RO_GENERIC_OFF),
    RO(RO_SHUFFLE_MOLEHILLS,            RO_GENERIC_OFF),
    RO(RO_SHUFFLE_MUMBO_TOKENS,         RO_GENERIC_OFF),
    RO(RO_SHUFFLE_MUSIC_NOTES,          RO_GENERIC_OFF),
    RO(RO_SHUFFLE_STOP_N_SWOP,          RO_GENERIC_OFF),
};
// clang-format on

} // namespace StaticData

} // namespace Rando
