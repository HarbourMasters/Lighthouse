#ifndef RANDO_TRAPS_H
#define RANDO_TRAPS_H

#include <cstdint>
#include <vector>
#include "port/Rando/Types.h"

namespace Rando {
namespace Traps {

// Swaps a share of the logic-free filler (blue eggs, extra lives, beehive honeycombs) for traps
void Place(std::vector<RandoSaveCheck>& pool, uint32_t seed);
// False while the player is in a state where this trap would be unsafe or do nothing
bool CanApply(RandoItemId randoItemId);
void Apply(RandoItemId randoItemId);

} // namespace Traps
} // namespace Rando

#endif // RANDO_TRAPS_H
