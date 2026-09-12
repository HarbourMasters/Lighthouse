#include "port/Prefs/Sections/RandoPrefs.h"

namespace Prefs::Rando {

// No Min/Max needed — a Bool has no invalid in-range state, which is half the point of the type.
Prefs::Bool Notifications{ SECTION_RANDO, "notifications", true,
                           Options<bool>().CVar("gRandoSettings.RandoNotifications") };

Prefs::String InputSeed{ SECTION_RANDO, "inputSeed", "", Options<std::string>().MaxSize(64) };

} // namespace Prefs::Rando
