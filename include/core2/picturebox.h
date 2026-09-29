#ifndef BANJO_KAZOOIE_CORE2_PICTUREBOX_H
#define BANJO_KAZOOIE_CORE2_PICTUREBOX_H

#include <ultra64.h>

void picturebox_func_8030C160(void);
void picturebox_func_8030C180(void);
void picturebox_init(void);
void picturebox_free(void);
void picturebox_spawn(void);
void picturebox_resetScissorBoxAndFramebuffer(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void picturebox_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
s16 *picturebox_getColorBuffer(void);
void picturebox_setScissorBox(void);

#endif
