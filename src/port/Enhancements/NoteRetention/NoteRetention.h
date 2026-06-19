#ifndef PORT_NOTE_RETENTION_H
#define PORT_NOTE_RETENTION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void port_noteRetention_beginMapLoad(int32_t mapId);
void port_noteRetention_onActorsFreed(void);

#ifdef __cplusplus
}
#endif

#endif
