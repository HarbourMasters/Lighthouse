#pragma once

#include <filesystem>
#include <functional>
#include <optional>

#include "ship/window/gui/FileBrowserWindow.h"

// Desktop platforms have a usable native file dialog (portable-file-dialogs). Consoles and Linux
// handhelds (arm) generally lack a native dialog / display server, so they fall back to
// libultraship's in-game ImGui file browser. Define LIGHTHOUSE_NATIVE_FILE_DIALOG to override.
#ifndef LIGHTHOUSE_NATIVE_FILE_DIALOG
#if defined(__SWITCH__) || defined(__WIIU__) || (defined(__linux__) && (defined(__aarch64__) || defined(__arm__)))
#define LIGHTHOUSE_NATIVE_FILE_DIALOG 0
#else
#define LIGHTHOUSE_NATIVE_FILE_DIALOG 1
#endif
#endif

namespace Lighthouse {

// Show a file picker described by @p request and deliver the chosen path (std::nullopt on cancel) to
// @p onResult.
//
//   - Native builds (desktop): a blocking portable-file-dialogs dialog. onResult fires synchronously
//     on the calling thread before PickFile returns.
//   - ImGui builds (consoles / arm-linux): libultraship's FileBrowserWindow. onResult fires later on
//     the render thread; PickFile returns immediately and is safe to call from any thread.
//
// Callers written for the async form (kick off, then poll a flag the callback sets) work with both
// backends unchanged: the native path just sets that flag before returning.
void PickFile(Ship::FileBrowserRequest request, std::function<void(std::optional<std::filesystem::path>)> onResult);

} // namespace Lighthouse
