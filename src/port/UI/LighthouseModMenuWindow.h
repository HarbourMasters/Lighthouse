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

// The Romhack Menu reuses the Mod Menu's enable/disable manager, but scoped to
// romhack overlays (the .o2r files carrying an aGameConfig under mods/~romhacks/).
// Enabling one here changes the Mod Menu's context to that hack's scoped/shared
// mods. Shares the underlying enabled/disabled lists with the Mod Menu.
class LighthouseRomhackMenuWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};
};

// Public so Engine.cpp can drive an initial scan before the GUI window
// initializes (so enabled mods are present in the ArchiveManager when the
// resource manager wakes up).
void UpdateModFiles(bool init = false, bool reset = false);

// True if `topLevelName` (a directory directly under mods/) is a romhack's
// scoped mod folder, i.e. it matches a discovered romhack overlay's basename.
// The loose-mod-directory loader uses this to skip scoped folders, which are
// handled by the mod scan instead. Valid only after UpdateModFiles() has run.
bool IsScopedModFolderName(const std::string& topLevelName);

void EnableMod(std::string file);
void DisableMod(std::string file);

// If UpdateModFiles(true) detected multiple enabled mods carrying
// assets/aGameConfig at boot, show an ImGui popup explaining that they were
// disabled to prevent runtime collisions. Must be called only after the
// modal window is initialized (i.e. after LighthouseGui::SetupGuiElements).
void MaybeShowModConflictPopup();

// If UpdateModFiles(true) refused to load romhack overlays because the base
// bk.o2r is not US v1.0, show an ImGui popup explaining they were disabled.
// Must be called only after the modal window is initialized.
void MaybeShowRomhackBaseMismatchPopup();

// Ensure the mod o2r named `keepBasename` is the only enabled overlay carrying
// assets/aGameConfig. Called right after an inline extraction succeeds so the
// boot-time conflict check doesn't quarantine the freshly-generated romhack.
void SetSoleEnabledRomhack(const std::string& keepBasename);

// Mod Menu "Generate Mod from ROM" button. Opens a ROM picker, extracts a slim
// mod o2r into the mods folder on a worker thread, then closes Lighthouse so
// the new archive loads at boot. Mirrors Starship's GenAssetFile menu flow.
void RequestInlineModExtraction();

// Mod Menu "Add Language Pack from ROM" button. Same flow as
// RequestInlineModExtraction, but runs Torch in dialog-pack mode so the result
// is a slim regional dialog overlay (mods/bk<region>.o2r) rather than a romhack.
void RequestInlineLanguagePackExtraction();

// Per-frame driver for RequestInlineModExtraction: renders the progress modal,
// services the custom-code prompt, and raises the completion popup. Called every
// frame from the always-visible modal window.
void DrawInlineModExtraction();

// True while an inline extraction worker thread is running. The main loop uses
// this to freeze the game and render GUI-only frames so the extractor isn't
// fighting a live 60fps game for the machine.
bool IsInlineModExtractionBusy();
#endif
