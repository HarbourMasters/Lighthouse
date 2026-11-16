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

#include "actor.h"
#include "functions.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"

extern "C" {
extern ActorInfo chBluePresentCollectible;
extern ActorInfo chGreenPresentCollectible;
extern ActorInfo chRedPresentCollectible;
}

#define ROMHACK_PORT_LIST(PORT) \
    PORT(Dreamie)               \
    PORT(JiggiesOfTime)         \
    PORT(NewHorizons)           \
    PORT(Nostalgia64)           \
    PORT(CutThroatCoast)        \
    PORT(BubblingBog)           \
    PORT(Gruntch)               \
    PORT(SnowGlowVillage)       \
    PORT(CheatosChallenges)     \
    PORT(LegendOfTheCrystalJiggy)

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

// spawnQueue_reset's case for the snow overlay.
void AddSnowSpawnableActors() {
    FP_addSpawnableActors();
    spawnableActorList_add(&chBluePresentCollectible, actor_new, ACTOR_FLAG_UNKNOWN_3);
    spawnableActorList_add(&chGreenPresentCollectible, actor_new, ACTOR_FLAG_UNKNOWN_3);
    spawnableActorList_add(&chRedPresentCollectible, actor_new, ACTOR_FLAG_UNKNOWN_3);
}

struct OverlaySpawnList {
    s32 overlay;
    void (*add)();
};

constexpr OverlaySpawnList kOverlaySpawnLists[] = {
    { OVERLAY_D_WITCH, lair_addSpawnableActors },     { OVERLAY_E_BATTLE, fight_addSpawnableActors },
    { OVERLAY_C_INTRO, cutscene_addSpawnableActors }, { OVERLAY_B_TRAINING, SM_addSpawnableActors },
    { OVERLAY_3_HAUNTED, MMM_addSpawnableActors },    { OVERLAY_2_WHALE, CC_addSpawnableActors },
    { OVERLAY_4_DESERT, GV_addSpawnableActors },      { OVERLAY_5_BEACH, TTC_addSpawnableActors },
    { OVERLAY_6_JUNGLE, MM_addSpawnableActors },      { OVERLAY_7_SWAMP, BGS_addSpawnableActors },
    { OVERLAY_8_SHIP, RBB_addSpawnableActors },       { OVERLAY_9_SNOW, AddSnowSpawnableActors },
    { OVERLAY_A_TREE, CCW_addSpawnableActors },
};

// Romhacks globalize the overlays: any actor can spawn on any map, so every overlay's actors join the spawnable list.
// spawnQueue_reset already added the loaded one's.
void AddOtherOverlaysSpawnableActors(IEvent* event) {
    const s32 loaded = ((OnSpawnableActorsReset*)event)->overlay;
    for (const OverlaySpawnList& list : kOverlaySpawnLists) {
        if (list.overlay != loaded) {
            list.add();
        }
    }
}

void RegisterRomhackPatches_Init() {
    COND_HOOK(OnSpawnableActorsReset, EVENT_PRIORITY_NORMAL, RomhackConfig_IsActive(), AddOtherOverlaysSpawnableActors);
    // Romhacks keep their learned moves when the stored state clears.
    COND_VB_SHOULD(VB_ABILITY_CLEAR_ALL, EVENT_PRIORITY_NORMAL, RomhackConfig_IsActive(), { *should = false; });

    const char* id = RomhackConfig_GetIdentifier();
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
