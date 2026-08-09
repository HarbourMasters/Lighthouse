#include <libultraship.h>
#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <fast/resource/type/Texture.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"
#include "port/UI/enhancementTypes.h"

#include <memory>

extern "C" {
#include "enums.h"
int port_getBootSequence(void);
void ResourceMgr_RegisterAssetOverride(uint32_t assetId, const char* customPath);
}

namespace {

constexpr const char* kXylophoneNintendoTex = "assets/model/ASSET_3B1_UNNAMED_tex_6";
constexpr const char* kXylophoneReplacementTex = "textures/intro/xylophone";
constexpr const char* kIntroCubeReplacementModel = "models/lighthouse";
constexpr uint32_t kAssetNintendoWalkAnim = 0x8F;
constexpr uint32_t kAssetNintendoShrugAnim = 0x90;
constexpr const char* kIntroCubeWalkAnim = "anim/lighthouse_walk";
constexpr const char* kIntroCubeShrugAnim = "anim/lighthouse_shrug";

bool bootSequenceIsDefault() {
    return port_getBootSequence() == BOOTSEQUENCE_DEFAULT;
}

void stripXylophoneNintendoLogo() {
    auto& rm = *Ship::Context::GetRawInstance()->GetResourceManager();
    if (!rm.GetArchiveManager()->HasFile(kXylophoneReplacementTex)) {
        return;
    }
    auto replacement = rm.LoadResource(kXylophoneReplacementTex);
    if (replacement != nullptr && std::static_pointer_cast<Fast::Texture>(replacement)->ImageData != nullptr) {
        rm.CacheExternalResource(kXylophoneNintendoTex, replacement);
    }
}

} // namespace

void RegisterIntroLogos_Init() {
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, bootSequenceIsDefault(), [](IEvent* event) {
        auto* ev = (OnMapLoad*)event;
        ResourceMgr_RegisterAssetOverride(ASSET_3A6_MODEL_INTRO_N64_CUBE, kIntroCubeReplacementModel);
        ResourceMgr_RegisterAssetOverride(kAssetNintendoWalkAnim, kIntroCubeWalkAnim);
        ResourceMgr_RegisterAssetOverride(kAssetNintendoShrugAnim, kIntroCubeShrugAnim);

        if (ev->nextMap == MAP_1E_CS_START_NINTENDO || ev->nextMap == MAP_1F_CS_START_RAREWARE) {
            stripXylophoneNintendoLogo();
        }
    });
}

static RegisterShipInitFunc introLogosInit(RegisterIntroLogos_Init, { CVAR_SETTING("BootSequence") });
