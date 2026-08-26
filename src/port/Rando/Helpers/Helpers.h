#pragma once
#include "port/Rando/Types.h"
#include <string>

namespace Rando {
namespace Helpers {

void SendNotification(RandoItemId randoItemId, const std::string& subject);

} // namespace Helpers
} // namespace Rando
