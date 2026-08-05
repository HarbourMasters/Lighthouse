// Registration hub for per-romhack ports.
//
// Each supported romhack lives in its own file under Romhack/Specific/ and
// exposes a Register<Hack>Patches() that installs its listeners. Registration
// itself is gated on the loaded romhack's RomhackTable.h identifier.
//
// To add a new port:
//   1. Add an entry in RomhackTable.h by hash.
//   2. Add Romhack/Specific/<Hack>.cpp exposing Register<Hack>Patches().
//   3. Add one PORT() line to the list below. The identifier must match the
//      RomhackTable.h identifier and the Register<Hack>Patches() name.

#include <cstring>

#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"

#define ROMHACK_PORT_LIST(PORT) \
    PORT(Dreamie)               \
    PORT(JiggiesOfTime)         \
    PORT(NewHorizons)           \
    PORT(Nostalgia64)           \
    PORT(CutThroatCoast)        \
    PORT(BubblingBog)           \
    PORT(Gruntch)               \
    PORT(SnowGlowVillage)

#define ROMHACK_PORT_DECL(name) void Register##name##Patches();
ROMHACK_PORT_LIST(ROMHACK_PORT_DECL)
#undef ROMHACK_PORT_DECL

namespace {

struct RomhackPort {
    const char* identifier;
    void (*install)();
};

constexpr RomhackPort kRomhackPorts[] = {
#define ROMHACK_PORT_ENTRY(name) { #name, &Register##name##Patches },
    ROMHACK_PORT_LIST(ROMHACK_PORT_ENTRY)
#undef ROMHACK_PORT_ENTRY
};

void RegisterRomhackPatches_Init() {
    const char* id = port_getRomhackIdentifier();
    if (id == nullptr) {
        return;
    }
    for (const RomhackPort& port : kRomhackPorts) {
        if (std::strcmp(id, port.identifier) == 0) {
            port.install();
            return;
        }
    }
}

RegisterShipInitFunc initFunc(RegisterRomhackPatches_Init, { "BOOT" });

} // namespace

#undef ROMHACK_PORT_LIST
