#ifndef BANJO_KAZOOIE_CORE1_EEPROM_H
#define BANJO_KAZOOIE_CORE1_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

s32 eeprom_writeBlocks(s32 file, s32 offset, void *buffer, s32 count);
s32 eeprom_readBlocks(s32 file, s32 offset, void *buffer, s32 count);

#ifdef __cplusplus
}
#endif

#endif
