#ifndef SAVE_CONVERTER_H
#define SAVE_CONVERTER_H

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
Result PickAndImport(int slot = kSlotAll);
Result PickAndExport(int slot = kSlotAll);

} // namespace SaveConverter

#endif // SAVE_CONVERTER_H
