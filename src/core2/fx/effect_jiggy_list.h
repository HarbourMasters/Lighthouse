// [port] effect_jiggy_list.c's typedef, moved here for SpawnJiggy.cpp.
#ifndef BANJO_KAZOOIE_CORE2_FX_EFFECT_JIGGY_LIST_H
#define BANJO_KAZOOIE_CORE2_FX_EFFECT_JIGGY_LIST_H

typedef struct {
    void (*init)(Struct81s *);
    void (*update)(Struct81s *);
    void (*draw)(Struct81s *, Gfx**, Mtx **, Vtx **);
    void (*bk_free)(Struct81s *);
    Struct81s unk10;
}Struct_core2_ABC00_0;

#endif
