#ifndef BANJO_KAZOOIE_CORE2_MAPSAVESTATE_H
#define BANJO_KAZOOIE_CORE2_MAPSAVESTATE_H

#include <ultra64.h>
#include "enums.h"

void mapSavestate_init(void);
void mapSavestate_clearAll(void);
void mapSavestate_defrag(void);
void mapSavestate_save(enum map_e map);
void mapSavestate_apply(enum map_e map);

#endif
