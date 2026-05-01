#pragma once

#include <libultraship/libultraship.h>

#ifdef __cplusplus
class LighthouseModMenuWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};

// Public so Engine.cpp can drive an initial scan before the GUI window
// initializes (so enabled mods are present in the ArchiveManager when the
// resource manager wakes up).
void UpdateModFiles(bool init = false, bool reset = false);

void EnableMod(std::string file);
void DisableMod(std::string file);

// If UpdateModFiles(true) detected multiple enabled mods carrying
// assets/aGameConfig at boot, show an ImGui popup explaining that they were
// disabled to prevent runtime collisions. Must be called only after the
// modal window is initialized (i.e. after LighthouseGui::SetupGuiElements).
void MaybeShowModConflictPopup();

// When Mod.PendingExtract is set (the user clicked "Generate Mod from ROM"
// and the next boot is about to extract a romhack), move every existing mod
// o2r that carries assets/aGameConfig from the enabled CVar list to the
// disabled list. Run once at boot before UpdateModFiles(true) so the
// freshly-extracted romhack ends up the sole enabled overlay (auto-enabled
// as a newcomer the boot after that). No-op if Mod.PendingExtract is unset.
void DisableConflictingModsForPendingExtract();
#endif
