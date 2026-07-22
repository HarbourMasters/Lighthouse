// Registration hub for per-romhack ports.
//
// Each supported romhack lives in its own file under Romhack/Specific/ and
// exposes a Register<Hack>Patches() that installs its listeners. Registration
// itself is gated on the loaded romhack's RomhackTable.h identifier, so vanilla
// and unrelated romhacks get no listeners installed at all.
//
// This runs from ShipInit::InitAll(), which GameEngine::Create calls after
// extraction and archive mounting, so the BKCF identifier is resolvable here.
//
// To add a new port:
//   1. Make sure the romhack has an entry in RomhackTable.h.
//   2. Add Romhack/Specific/<Hack>.cpp exposing Register<Hack>Patches().
//   3. Dispatch to it from RegisterRomhackPatches_Init() below.

#include <cstring>

#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"

void RegisterCutThroatCoastPatches();
void RegisterJiggiesOfTimePatches();
void RegisterNewHorizonsPatches();
void RegisterNostalgia64Patches();

namespace {

void RegisterRomhackPatches_Init() {
    const char* id = port_getRomhackIdentifier();
    if (id == nullptr) {
        return;
    }

    if (std::strcmp(id, "JiggiesOfTime") == 0) {
        RegisterJiggiesOfTimePatches();
    } else if (std::strcmp(id, "NewHorizons") == 0) {
        RegisterNewHorizonsPatches();
    } else if (std::strcmp(id, "Nostalgia64") == 0) {
        RegisterNostalgia64Patches();
    } else if (std::strcmp(id, "CutThroatCoast") == 0) {
        RegisterCutThroatCoastPatches();
    }
}

RegisterShipInitFunc initFunc(RegisterRomhackPatches_Init, { "BOOT" });

} // namespace
