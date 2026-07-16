#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

/**
 * DISABLE_ANCHOR
 *
 * No current use, potentially will be used for a future feature.
 */

void Anchor::HandlePacket_DisableAnchor(nlohmann::json& payload) {
    Disable();
}
