#ifndef SAVE_CONVERTER_H
#define SAVE_CONVERTER_H

#include <functional>
#include <string>

namespace SaveConverter {

constexpr int kSlotAll = 0;

struct Result {
    bool ok = false;
    int filesImported = 0;
    std::string message;
};

Result ImportFromRawEeprom(const std::string& srcPath, int slot = kSlotAll);
Result ExportToRecompBin(const std::string& dstPath, int slot = kSlotAll);

// Open the file picker, then import/export on the chosen path. onComplete runs when done; an empty
// Result.message means the user cancelled.
void PickAndImport(int slot, std::function<void(Result)> onComplete);
void PickAndExport(int slot, std::function<void(Result)> onComplete);

} // namespace SaveConverter

#endif // SAVE_CONVERTER_H
