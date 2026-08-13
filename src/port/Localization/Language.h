#ifndef LIGHTHOUSE_LANGUAGE_H
#define LIGHTHOUSE_LANGUAGE_H

#include <string>
#include <utility>
#include <vector>

// The language-pack system: detects the base game's language(s), discovers
// language packs from their "langinfo", tracks the active selection, and builds
// the dialog override that re-points the decomp's v1.0 asset ids at a pack. It
// drives the resource layer through ResourceHelpers_ApplyLanguage.
namespace Lighthouse {

// Rebuilds the available-language list from the base game's region plus every
// loaded language pack's "langinfo".
void RescanLanguages();

// Display names of all currently available dialog languages (base + packs),
// e.g. "English (US)", "English (UK)", "French".
std::vector<std::string> GetAvailableLanguageNames();

// Activates a language by display name: re-points dialog to its source archive,
// sets the dialog index, and evicts cached dialog so the change takes effect on
// the next dialog. No-op if the name isn't currently available.
void SetActiveLanguage(const std::string& name);

// Number of available dialog languages (base + packs).
int GetAvailableLanguageCount();

// (LanguageKey, displayName) pairs for building the language combobox.
std::vector<std::pair<int32_t, const char*>> GetLanguageComboEntries();

// Stable key for a language name. Used as the imgui combobox key and persisted
// CVar value so the selection survives reboots and changes to the pack list.
int32_t LanguageKey(const std::string& name);

} // namespace Lighthouse

#endif // LIGHTHOUSE_LANGUAGE_H
