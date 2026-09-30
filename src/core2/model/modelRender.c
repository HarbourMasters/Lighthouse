// BanjoDecomp: core2/modelRender.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "core2/modelRender.h"
#include "animation.h"

#include "libultraship/libultra/gbi.h"

#include "port/Patches/Patches.h"
#include "port/Interpolation/FrameInterpolation.h"
#include "port/Patches/GeoCull.h"

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

extern bool cameraAreaList_searchForEntryInBounds(BKCameraAreaList *this, u8 *id, u32 count);
extern void cameraAreaList_updateInBoundsFlag(BKCameraAreaList *, f32[3], f32);
extern void mlMtxRotatePYR(f32, f32, f32);
extern void assetCache_free(void *);
extern AnimMtxList *animMtxList_new();
extern AnimMtxList *animMtxList_defrag(AnimMtxList *);
extern MtxF *animMtxList_get(AnimMtxList *this, s32 arg1);
extern void lighthouse_setFrustumChecksEnabled(bool enabled);


typedef struct{
    void (* unk0)(Actor *);
    Actor *unk4;
} Struct_Core2_B1400_1;


typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8;
    s16 unkA;
    f32 unkC[3];
}GeoCmd0;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    f32 unk8[3];
    f32 unk14[3];
    s16 unk20;
    s16 unk22;
    s32 unk24;
}GeoCmd1;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    u8  unk8;
    s8  unk9;
}GeoCmd2;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8;
}GeoCmd3;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8[];
}GeoCmd5;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s32 unk8;
}GeoCmd6;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    u8  pad8;
    s16 unkA;
}GeoCmd7;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    f32 max_8;
    f32 min_C;
    f32 unk10[3];
    s32 subgeo_offset_1C;
}GeoCmd8;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8;
    s16 unkA;
    f32 unkC[3];
}GeoCmdA;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8;
    s16 unkA;
    s32 unkC[];
}GeoCmdC;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8[3];
    s16 unkE[3];
    s16 unk14;
}GeoCmdD;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8[3];
    s16 unkE;
    s16 unk10;
    s16 unk12;
}GeoCmdE;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s16 unk8;
    u8  unkA;
    u8  unkB;
    u8  unkC[12];
}GeoCmdF;

typedef struct {
    s32 cmd_0;
    s32 size_4;
    s32 unk8;
}GeoCmd10;


void modelRender_geoCmd_Unk0(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_SORT(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_BONE(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_LOADDL(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_NOP(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_SKINNING(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_CALL(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_LOADDL2(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_NOP(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_TEXWRAP(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_LOD(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_REFPOINT(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_SELECTOR(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_DRAWDIST(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_UnkE(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_geoCmd_CAMERA(Gfx **, Mtx **, struct bk_geo_cmd_s *);
void modelRender_executeGeoCmds(Gfx **, Mtx **, BKGeoCmd *);
void modelRender_setAppendageVisibility(s32 arg0, s32 arg1);

// Sets up 2 cycle mode
Gfx setup2CycleDL[] =
{
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_1PRIMITIVE),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x80),
    gsSPEndDisplayList()
};

// Sets up 2 cycle mode with black prim color
Gfx setup2CycleBlackPrimDL[] =
{
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_1PRIMITIVE),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetPrimColor(0, 0, 0x00, 0x00, 0x00, 0x00),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x80),
    gsSPEndDisplayList()
};

// Sets up 2 cycle mode with white env color
Gfx setup2CycleWhiteEnvDL[] =
{
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_1PRIMITIVE),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xFF),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x80),
    gsSPEndDisplayList()
};

// Sets up 2 cycle mode (duplicate of setup2CycleDL)
Gfx setup2CycleDL_copy[] =
{
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_1PRIMITIVE),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x80),
    gsSPEndDisplayList()
};

// List of render modes
Gfx renderModesNoDepthOpa[][2] = {
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_OPA_SURF2),
        gsSPEndDisplayList()
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, CVG_DST_SAVE | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    }
};

// Same as renderModesNoDepthOpa but with Z_CMP | Z_UPD added to the first 6 and Z_CMP added to the next 7
Gfx renderModesFullDepthOpa[][2] = {
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_OPA_SURF2),
        gsSPEndDisplayList()
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_AA_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList()
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | CVG_DST_SAVE | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList()
    }
};

// Same as renderModesNoDepthOpa but with Z_CMP added to all entries
Gfx renderModesDepthCompareOpa[][2] = {
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_OPA_SURF2),
        gsSPEndDisplayList()
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_OPA_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | CVG_DST_SAVE | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    }
};

// Same as renderModesNoDepthOpa but with OPA replaced by XLU
Gfx renderModesNoDepthXlu[][2] = {
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, CVG_DST_SAVE | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    }
};

// Same as renderModesFullDepthOpa but with OPA replaced by XLU
Gfx renderModesFullDepthXlu[][2] = {
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_XLU_SURF2),
        gsSPEndDisplayList()
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | Z_UPD | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList()
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | CVG_DST_SAVE | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList()
    }
};

// Same as renderModesDepthCompareOpa but with OPA replaced by XLU
Gfx renderModesDepthCompareXlu[][2] = {
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList()
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        gsDPSetRenderMode(G_RM_PASS, Z_CMP | CVG_DST_SAVE | G_RM_AA_XLU_SURF2),
        gsSPEndDisplayList(),
    }
};

// Mipmap tile configuration with no wrapping (G_TX_NOMASK disables wrap)
Gfx mipMapClampDL[] = 
{
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0000, 2, 0, G_TX_NOMIRROR, G_TX_NOMASK, 0, G_TX_NOMIRROR, G_TX_NOMASK, 0),
    gsDPSetTileSize(2, 0, 0, (32 - 1) << 2, (32 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0100, 3, 0, G_TX_NOMIRROR, G_TX_NOMASK, 1, G_TX_NOMIRROR, G_TX_NOMASK, 1),
    gsDPSetTileSize(3, 0, 0, (16 - 1) << 2, (16 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0104, 4, 0, G_TX_NOMIRROR, G_TX_NOMASK, 2, G_TX_NOMIRROR, G_TX_NOMASK, 2),
    gsDPSetTileSize(4, 0, 0, (8 - 1) << 2, (8 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0106, 5, 0, G_TX_NOMIRROR, G_TX_NOMASK, 3, G_TX_NOMIRROR, G_TX_NOMASK, 3),
    gsDPSetTileSize(5, 0, 0, (4 - 1) << 2, (4 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0107, 6, 0, G_TX_NOMIRROR, G_TX_NOMASK, 4, G_TX_NOMIRROR, G_TX_NOMASK, 4),
    gsDPSetTileSize(6, 0, 0, (2 - 1) << 2, (2 - 1) << 2),
    gsSPEndDisplayList()
};

// Mipmap tile configuration with wrapping
Gfx mipMapWrapDL[] =
{
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0000, 2, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, 0),
    gsDPSetTileSize(2, 0, 0, (32 - 1) << 2, (32 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0100, 3, 0, G_TX_NOMIRROR | G_TX_WRAP, 4, 1, G_TX_NOMIRROR | G_TX_WRAP, 4, 1),
    gsDPSetTileSize(3, 0, 0, (16 - 1) << 2, (16 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0104, 4, 0, G_TX_NOMIRROR | G_TX_WRAP, 3, 2, G_TX_NOMIRROR | G_TX_WRAP, 3, 2),
    gsDPSetTileSize(4, 0, 0, (8 - 1) << 2, (8 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0106, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, 2, 3, G_TX_NOMIRROR | G_TX_WRAP, 2, 3),
    gsDPSetTileSize(5, 0, 0, (4 - 1) << 2, (4 - 1) << 2),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x0107, 6, 0, G_TX_NOMIRROR | G_TX_WRAP, 1, 4, G_TX_NOMIRROR | G_TX_WRAP, 1, 4),
    gsDPSetTileSize(6, 0, 0, (2 - 1) << 2, (2 - 1) << 2),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTextureLOD(G_TL_LOD),
    gsSPEndDisplayList()
};

s32 D_80370990 = 0;
s32 cur_model_would_have_been_culled_in_demo = 0;

BKGeoCmdFunc sGeoCmdList[] = {
    modelRender_geoCmd_Unk0,
    modelRender_geoCmd_SORT,
    modelRender_geoCmd_BONE,
    modelRender_geoCmd_LOADDL,
    modelRender_geoCmd_NOP, //empty_4
    modelRender_geoCmd_SKINNING,
    modelRender_geoCmd_CALL,
    modelRender_geoCmd_LOADDL2,
    modelRender_geoCmd_LOD,
    modelRender_geoCmd_NOP, //empty_9
    modelRender_geoCmd_REFPOINT,
    modelRender_geoCmd_NOP, //empty_B
    modelRender_geoCmd_SELECTOR,
    modelRender_geoCmd_DRAWDIST,
    modelRender_geoCmd_UnkE,
    modelRender_geoCmd_CAMERA,
    modelRender_geoCmd_TEXWRAP
};

enum model_render_color_mode_e{
    COLOR_MODE_DYNAMIC_PRIM_AND_ENV,
    COLOR_MODE_DYNAMIC_ENV,
    COLOR_MODE_STATIC_OPAQUE,
    COLOR_MODE_STATIC_TRANSPARENT
};

/* .bss */
Vec3fArray *modelRenderRefPoints;
s32  D_80383658[0x2A];
BoneTransformList *modelRenderBoneTransformList;
bool D_80383704;
f32  D_80383708;
f32  D_8038370C;
s32  D_80383710;
enum model_render_color_mode_e  modelRenderColorMode;
BKGfxList *            modelRenderDisplayList;
AnimMtxList *            D_8038371C;
static BKTextureList * modelRenderTextureList;
s32                    modelRenderAnimatedTexturesCacheId;
static BKVertexList *  modelRendervertexList;
BKCameraAreaList *     modelRenderCameraAreaList;
AnimMtxList *            modelRenderAnimMtxList;
f32                    modelRenderScale;

struct{
    s32 env[4];
    s32 prim[4];
} modelRenderDynColors;

struct{
    f32 unk0[3];
    f32 unkC[3];
    s32 unk18;
    f32 unk1C[3];
    f32 unk28[3];
} D_80383758;

struct{
    model_render_pre_draw_callback_f pre_draw;
    void *pre_draw_arg;
    model_render_post_draw_callback_f post_draw;
    void *post_draw_arg;
} modelRenderCallback;

s32 modelRenderDynEnvColor[4];

struct {
    s32 unk0;
    f32 unk4[3];
}D_803837B0;

u8 modelRenderDynAlpha;

struct {
    s32 model_id; //model_asset_index
    f32 unk4;
    f32 unk8;
    u8 padC[0x4];
} sSecondaryModelData; 

static enum model_render_depth_mode_e modelRenderDepthMode;

struct {
    LookAt lookat_buffer[32];
    LookAt *cur_lookat;
    LookAt *lookat_buffer_end;
    f32 eye_pos[3];
} D_803837E0;
MtxF D_80383BF8;
f32 modelRenderCameraPosition[3];
f32 modelRenderCameraRotation[3];
BKModelBin *modelRenderModelBin;
f32 modelRenderRotation[3];
f32 D_80383C64;
f32 D_80383C68[3];
f32 D_80383C78[3];
f32 D_80383C88[3];
f32 transformed_pos[3];

/* .code */
void modelRender_reset(void){
    modelRenderBoneTransformList = 0;
    D_80383708 = 30000.0f;
    D_80383704 = true;
    D_8038370C = 1.0f;
    D_80383710 = false;
    modelRenderColorMode = COLOR_MODE_STATIC_OPAQUE;
    modelRenderRefPoints = 0;
    modelRenderDisplayList = NULL;
    D_8038371C = NULL;
    modelRenderTextureList = NULL;
    modelRenderAnimatedTexturesCacheId = 0;
    modelRendervertexList = NULL;
    modelRenderCameraAreaList = 0;
    modelRenderCallback.pre_draw = NULL;
    modelRenderCallback.post_draw = NULL;
    D_803837B0.unk0 = 0;
    sSecondaryModelData.model_id = 0;
    modelRenderDepthMode = MODEL_RENDER_DEPTH_NONE;
    modelRender_setAppendageVisibility(1,1);
    modelRender_setAppendageVisibility(2,0);
    if(D_80383758.unk18){
        viewport_setPosition_vec3f(D_80383758.unk1C);
        viewport_setRotation_vec3f(D_80383758.unk28);
        viewport_update();
    }
}

//empty cmd, 
void modelRender_geoCmd_NOP(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    return;
}

//cmd0_???
void modelRender_geoCmd_Unk0(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd0 *cmd = (GeoCmd0 *)arg2;
    f32 sp30[3];

    if(cmd->unk8){
        FrameInterpolation_RecordOpenChildHash3("billboard", (uintptr_t)cmd, 0, 0);
        mlMtx_apply_vec3f(sp30, cmd->unkC);
        mlMtx_push_translation(sp30[0], sp30[1], sp30[2]);
        mlMtxRotYaw(modelRenderCameraRotation[1]);
        if(!cmd->unkA){
            mlMtxRotPitch(modelRenderCameraRotation[0]);
        }
        mlMtxScale(modelRenderScale);
        mlMtxTranslate(-cmd->unkC[0], -cmd->unkC[1], -cmd->unkC[2]);
        mlMtxApply(*mtx);
        gSPMatrix((*gfx)++, (*mtx)++, G_MTX_PUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk8));
        mlMtxPop();
        gSPPopMatrix((*gfx)++, G_MTX_MODELVIEW);
        FrameInterpolation_RecordCloseChild();
    }
}

//cmd1_SORT
void modelRender_geoCmd_SORT(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd1 *cmd = (GeoCmd1 *)arg2;
    f32 f14;
    s32 tmp_v0;

    mlMtx_apply_vec3f(D_80383C78, cmd->unk8);
    mlMtx_apply_vec3f(D_80383C88, cmd->unk14);

    D_80383C68[0] = D_80383C88[0] - D_80383C78[0];
    D_80383C68[1] = D_80383C88[1] - D_80383C78[1];
    D_80383C68[2] = D_80383C88[2] - D_80383C78[2];

    f14 = D_80383C68[0]*D_80383C78[0] + D_80383C68[1]*D_80383C78[1] + D_80383C68[2]*D_80383C78[2];
    f14 = -f14;
    // [port] Disable Culling SORT one-sided geometry fix
    // A one-sided SORT node normally submits only the camera-facing child.
    // With Disable Culling, submit both children in camera order during normal gameplay.
    // Demo/playback keeps the original branch selection for deterministic behavior.
    if (port_shouldDisableCulling() && !port_isDemoPlayback() && (cmd->unk20 & 1)) {
        D_80383C64 = f14;
        if (0.0f <= f14) {
            if (cmd->unk22) {
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk22));
            }
            if (cmd->unk24) {
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk24));
            }
        } else {
            if (cmd->unk24) {
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk24));
            }
            if (cmd->unk22) {
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk22));
            }
        }
        return;
    }
    if(cmd->unk20 & 1){
        if(0.0f <= f14 && (tmp_v0 = cmd->unk24)){
            D_80383C64 = f14;
            modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + tmp_v0));
        }
        else{
            D_80383C64 = f14;
            if(f14 < 0.0f){
                if(cmd->unk22)
                    modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk22));
            }
        }
    }
    else{
        D_80383C64 = f14;
        if(0.0f <= f14){
            if(cmd->unk22)
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk22));

            if(cmd->unk24)
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk24));
        }
        else{
            if(cmd->unk24)
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk24));

            if(cmd->unk22)
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk22));
        }
    }
}

//cmd10_???
void modelRender_geoCmd_TEXWRAP(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd10 *cmd = (GeoCmd10 *)arg2;

    switch(cmd->unk8){
        case 1:
            gSPDisplayList((*gfx)++, mipMapClampDL);
            break;
        case 2:
            gSPDisplayList((*gfx)++, mipMapWrapDL);
            break;
    }
}

//cmd2_BONE
void modelRender_geoCmd_BONE(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd2 *cmd = (GeoCmd2 *)arg2;

    // [port] Stable per-bone scope.
    FrameInterpolation_RecordOpenChildHash3("bone", (uintptr_t)(u8)cmd->unk9, (uintptr_t)cmd, 0);
    if(D_8038371C){
        mlMtx_push_multiplied_2(&D_80383BF8, animMtxList_get(D_8038371C, cmd->unk9));
        if(D_80370990){
            mlMtxApply(*mtx);
            gSPMatrix((*gfx)++, (*mtx)++, G_MTX_PUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        }
    }
    if(cmd->unk8){
        modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk8));
    }
    if(D_8038371C){
        mlMtxPop();
        if(D_80370990){
            gSPPopMatrix((*gfx)++, G_MTX_MODELVIEW);
        }
    }
    FrameInterpolation_RecordCloseChild();
}

//cmd3_LOAD_DL
void modelRender_geoCmd_LOADDL(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd3 *cmd = (GeoCmd3 *)arg2;
    Gfx *vptr;

    if(D_80370990){
        vptr = &modelRenderDisplayList->list[cmd->unk8];
        gSPDisplayList((*gfx)++, (Gfx *)osVirtualToPhysical(vptr));
    }
}

//Cmd5_SKINNING
void modelRender_geoCmd_SKINNING(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd5 *cmd = (GeoCmd5 *)arg2;
    int i;

    if(D_80370990){
        gSPDisplayList((*gfx)++, (Gfx *)osVirtualToPhysical(modelRenderDisplayList->list + cmd->unk8[0]));
    }

    if(D_80370990){
        for(i = 1; cmd->unk8[i]; i++){
            // [port] Per-cluster scope; i binds to the same deformation
            // region across frames.
            FrameInterpolation_RecordOpenChild("skin", (uintptr_t)i);
            mlMtxApply(*mtx);
            gSPMatrix((*gfx)++, (*mtx)++, G_MTX_PUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList((*gfx)++, (Gfx *)osVirtualToPhysical(modelRenderDisplayList->list + cmd->unk8[i]));
            FrameInterpolation_RecordCloseChild();
        }
    }
}

//Cmd6_???
void modelRender_geoCmd_CALL(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd6 *cmd = (GeoCmd6 *)arg2;
    modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk8));
}

//Cmd7_LOAD_DL???
void modelRender_geoCmd_LOADDL2(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    if(D_80370990){
        gSPDisplayList((*gfx)++, (Gfx *)osVirtualToPhysical(modelRenderDisplayList->list + ((GeoCmd7*)arg2)->unkA));
    }
}

//Cmd8_LOD
void modelRender_geoCmd_LOD(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmd8 *cmd = (GeoCmd8 *)arg2;
    f32 dist;

    if(cmd->subgeo_offset_1C){
        s32 draw;
        if(port_shouldDisableLOD()){
            dist = 1.0f;
        } else {
            mlMtx_apply_vec3f(transformed_pos, cmd->unk10);
            dist = gu_sqrtf(transformed_pos[0]*transformed_pos[0] + transformed_pos[1]*transformed_pos[1] + transformed_pos[2]*transformed_pos[2]);
        }
        draw = (cmd->min_C < dist && dist <= cmd->max_8);
        draw = port_geoCullDraw(OCCLUSION_CMD_LOD, cmd, modelRenderModelBin, draw, NULL, 0, (s32)cmd->min_C, (s32)cmd->max_8);
        if(draw){
            modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->subgeo_offset_1C));
        }
    }
}

//CmdA_REFERENCE_POINT
void modelRender_geoCmd_REFPOINT(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmdA *cmd = (GeoCmdA *)arg2;
    f32 sp20[3];

    if(modelRenderRefPoints){
        if(D_8038371C){
            mlMtx_push_multiplied_2(&D_80383BF8, animMtxList_get(D_8038371C, cmd->unkA));
            mlMtx_apply_vec3f(sp20, cmd->unkC);
            mlMtxPop();
        }
        else{
            mlMtx_apply_vec3f(sp20, cmd->unkC);
        }
        sp20[0] += modelRenderCameraPosition[0];
        sp20[1] += modelRenderCameraPosition[1];
        sp20[2] += modelRenderCameraPosition[2];
        vec3fArray_set_vec3f(modelRenderRefPoints, cmd->unk8, sp20);
    }
}

//CmdC_SELECTOR
void modelRender_geoCmd_SELECTOR(Gfx **gfx, Mtx **mtx, struct bk_geo_cmd_s *arg2){
    GeoCmdC *cmd = (GeoCmdC *) arg2;
    uintptr_t sub_cmd;
    s32 indx;
    s32 s2;
    s32 s1;
    s32 *s0;

    indx = D_80383658[cmd->unkA];

    if (cmd->unkA == 0)
        return;

    if (indx == 0)
        return;
    
    if (0 < indx) {
        if (indx <= cmd->unk8) {
            s0 = cmd->unkC;
            sub_cmd = (uintptr_t)cmd;
            sub_cmd += *(s32*)(s0 + (indx - 1));
            modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)sub_cmd);
        }
    } else {
        s1 = indx * (-1);
        s0 = cmd->unkC;
        for (s2 = 0; s2 < cmd->unk8; s2++) {
            if (s1 & 1)
            {
                sub_cmd = (uintptr_t)cmd;
                sub_cmd += s0[0];
                modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)sub_cmd);
            }
            s1 >>= 1;
            s0++;
        }
    }
}

//CmdD_DRAW_DISTANCE
extern f32 GameEngine_GetAspectRatio(void);
void modelRender_geoCmd_DRAWDIST(Gfx ** gfx, Mtx ** mtx, struct bk_geo_cmd_s *arg2){
    f32 sp2C[3];
    f32 sp20[3];
    GeoCmdD * cmd = (GeoCmdD *)arg2;
    if(cmd->unk14){
        sp2C[0] = (f32)cmd->unk8[0] * modelRenderScale;
        sp2C[1] = (f32)cmd->unk8[1] * modelRenderScale;
        sp2C[2] = (f32)cmd->unk8[2] * modelRenderScale;
        sp20[0] = (f32)cmd->unkE[0] * modelRenderScale;
        sp20[1] = (f32)cmd->unkE[1] * modelRenderScale;
        sp20[2] = (f32)cmd->unkE[2] * modelRenderScale;
        // [port] The N64 bounding boxes in CmdD_DRAW_DISTANCE are too conservative
        // for the port's viewport (292x216 -> 320x240 at 4:3). Extend to all aspect
        // ratios since the port always renders at a higher effective resolution.
        if (port_shouldDisableCulling() || EventSystem_Should(VB_DRAWDIST_BOX_CULL, true, sp2C, sp20)) {
            modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk14));
        }
    }
}

//cmdE_???
void modelRender_geoCmd_UnkE(Gfx ** gfx, Mtx ** mtx, struct bk_geo_cmd_s *arg2){
    f32 sp34[3];
    f32 sp30;
    GeoCmdE * cmd = (GeoCmdE *)arg2;

    if(cmd->unk12 == -1){
        s32 draw;
        sp34[0] = (f32)cmd->unk8[0] * modelRenderScale;
        sp34[1] = (f32)cmd->unk8[1] * modelRenderScale;
        sp34[2] = (f32)cmd->unk8[2] * modelRenderScale;
        sp30 = (f32)cmd->unkE*modelRenderScale;
        draw = (viewport_func_8024DB50(sp34, sp30) && cmd->unk10) ? 1 : 0;
        draw = port_geoCullDraw(OCCLUSION_CMD_UNKE, cmd, modelRenderModelBin, draw, NULL, 0, 0, 0) && cmd->unk10;
        if(draw){
            modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk10));
        }
    }
    else{
        s32 draw;
        sp34[0] = (f32)cmd->unk8[0];
        sp34[1] = (f32)cmd->unk8[1];
        sp34[2] = (f32)cmd->unk8[2];

        sp30 = (f32)cmd->unkE*modelRenderScale;
        if(D_8038371C){
            mlMtx_push_multiplied_2(&D_80383BF8, animMtxList_get(D_8038371C, cmd->unk12));
            mlMtx_apply_vec3f(sp34, sp34);
            mlMtxPop();
        }
        else{
            mlMtx_apply_vec3f(sp34, sp34);
        }

        sp34[0] += modelRenderCameraPosition[0];
        sp34[1] += modelRenderCameraPosition[1];
        sp34[2] += modelRenderCameraPosition[2];
        draw = (viewport_func_8024DB50(sp34, sp30) && cmd->unk10) ? 1 : 0;
        draw = port_geoCullDraw(OCCLUSION_CMD_UNKE, cmd, modelRenderModelBin, draw, NULL, 0, 0, 0) && cmd->unk10;
        if(draw){
            modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk10));
        }

    }

}

//cmdF_??? (processes model_setup offset_0x20)
void modelRender_geoCmd_CAMERA(Gfx ** gfx, Mtx ** mtx, struct bk_geo_cmd_s *arg2){
    GeoCmdF *cmd = (GeoCmdF *)arg2;
    int tmp_v0 = cameraAreaList_searchForEntryInBounds(modelRenderCameraAreaList, cmd->unkC, cmd->unkA);
    int draw = (!tmp_v0 && (cmd->unkB & 1)) || (tmp_v0 && (cmd->unkB & 2));
    draw = port_geoCullDraw(OCCLUSION_CMD_CAMERA, cmd, modelRenderModelBin, draw, cmd->unkC, cmd->unkA, cmd->unkB, 0);
    if (draw) {
        if(cmd->unk8 != 0)
            modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd*)((u8*)cmd + cmd->unk8));
    }

}

//render_GeoList
void modelRender_executeGeoCmds(Gfx ** gfx, Mtx ** mtx, BKGeoCmd *geo_list){
    do{
        // [port] bounds check geo command to prevent null function pointer crash
        if ((s32)geo_list->cmd < 0 || (s32)geo_list->cmd >= (s32)(sizeof(sGeoCmdList)/sizeof(sGeoCmdList[0]))) {
            if(geo_list->next_offset == 0) return;
            geo_list = (BKGeoCmd*)((u8*)geo_list + geo_list->next_offset);
            continue;
        }
        sGeoCmdList[geo_list->cmd](gfx, mtx, geo_list);
        if(geo_list->next_offset == 0)
            return;
        geo_list = (BKGeoCmd*)((u8*)geo_list + geo_list->next_offset);
    }while(1);
}

BKModelBin *modelRender_draw(Gfx **gfx, Mtx **mtx, f32 position[3], f32 rotation[3], f32 scale, f32*arg5, BKModelBin* model_bin){

    f32 camera_focus[3];
    f32 camera_focus_distance;
    f32 padEC;
    f32 object_position[3];
    void *rendermode_table_opa; // Table of render modes to use for opaque rendering
    void *rendermode_table_xlu; // Table of render modes to use for translucent rendering
    f32 spD4;
    f32 spD0;
    BKVertexList *verts;
    s32 alpha; 
    f32 tmp_f0;
    f32 padB8;

    cur_model_would_have_been_culled_in_demo = false;
    
    if( (!model_bin && !sSecondaryModelData.model_id)
        || (model_bin && sSecondaryModelData.model_id)
    ){

        modelRender_reset();
        return 0;
    }

    // [port] Extended draw distance: scale/extend the model cull, keep the fade flag.
    port_applyModelDrawDistanceCull(&D_80383710, &D_8038370C, &D_80383708);

    D_80370990 = 0;
    viewport_getPosition_vec3f(modelRenderCameraPosition);
    viewport_getRotation_vec3f(modelRenderCameraRotation);
    if(D_80383758.unk18){
        D_80383758.unk1C[0] = modelRenderCameraPosition[0];
        D_80383758.unk1C[1] = modelRenderCameraPosition[1];
        D_80383758.unk1C[2] = modelRenderCameraPosition[2];

        D_80383758.unk28[0] = modelRenderCameraRotation[0];\
        D_80383758.unk28[1] = modelRenderCameraRotation[1];\
        D_80383758.unk28[2] = modelRenderCameraRotation[2];
    }

    if(position){
        object_position[0] = position[0];
        object_position[1] = position[1];
        object_position[2] = position[2];
    }
    else{
        object_position[0] = object_position[1] = object_position[2] = 0.0f;
    }

    camera_focus[0] = object_position[0] - modelRenderCameraPosition[0];
    camera_focus[1] = object_position[1] - modelRenderCameraPosition[1];
    camera_focus[2] = object_position[2] - modelRenderCameraPosition[2];

    // [port] Disable Culling model distance gates
    if ((!port_shouldDisableCulling() || port_isDemoPlayback()) && ( ((camera_focus[0] < -17000.0f) || (17000.0f < camera_focus[0]))
        || ((camera_focus[1] < -17000.0f) || (17000.0f < camera_focus[1]))
        || ((camera_focus[2] < -17000.0f) || (17000.0f < camera_focus[2]))
    )) {
        modelRender_reset();
        return 0;
    }

    if(D_80383758.unk18){
        modelRenderCameraPosition[0] = D_80383758.unk0[0];
        modelRenderCameraPosition[1] = D_80383758.unk0[1];
        modelRenderCameraPosition[2] = D_80383758.unk0[2];

        modelRenderCameraRotation[0] = D_80383758.unkC[0],
        modelRenderCameraRotation[1] = D_80383758.unkC[1],
        modelRenderCameraRotation[2] = D_80383758.unkC[2];
        viewport_setPosition_vec3f(modelRenderCameraPosition);
        viewport_setRotation_vec3f(modelRenderCameraRotation);
        viewport_update();
        camera_focus[0] = object_position[0] - modelRenderCameraPosition[0];
        camera_focus[1] = object_position[1] - modelRenderCameraPosition[1];
        camera_focus[2] = object_position[2] - modelRenderCameraPosition[2];
    }

    if(model_bin){
        verts = modelRendervertexList ? modelRendervertexList : (BKVertexList *)((uintptr_t)model_bin + model_bin->vtx_list_offset);
        spD0 = verts->global_norm;
        spD4 = verts->local_norm;
    }
    else{
        spD0 = sSecondaryModelData.unk8;
        spD4 = sSecondaryModelData.unk4;
    }
    camera_focus_distance = gu_sqrtf(camera_focus[0]*camera_focus[0] + camera_focus[1]*camera_focus[1] + camera_focus[2]*camera_focus[2]);
    if( 4000.0f <= camera_focus_distance && spD4*scale*D_8038370C*50.0f < D_80383708){
        D_80383708 = spD4*scale*D_8038370C*50.0f;
    }

    if ((!port_shouldDisableCulling() || port_isDemoPlayback()) && D_80383708 <= camera_focus_distance) {
        modelRender_reset();
        return 0;
    }

    if (port_shouldDisableCulling()) {
        if (port_isDemoPlayback()) {
            // Exact widescreen demo state: remember original visibility,
            // but keep the visual model alive during the draw.
            cur_model_would_have_been_culled_in_demo =
                !((D_80383704) ? viewport_func_8024DB50(object_position, spD0 * scale) : 1);
        } else {
            cur_model_would_have_been_culled_in_demo = false;
        }
        D_80370990 = true;
    } else {
        D_80370990 = (D_80383704) ? viewport_func_8024DB50(object_position, spD0*scale) : 1;
        if(D_80370990 == 0){
            modelRender_reset();
            return 0;
        }
    }

    if(modelRenderCallback.pre_draw != NULL){
        modelRenderCallback.pre_draw(modelRenderCallback.pre_draw_arg);
    }
    func_80349AD0();
    if(model_bin == NULL){
        model_bin = assetcache_get(sSecondaryModelData.model_id);
    }
    modelRenderModelBin = model_bin;
    modelRenderDisplayList = modelRenderDisplayList ? modelRenderDisplayList : (BKGfxList *)((uintptr_t)modelRenderModelBin + (uintptr_t)(u32)modelRenderModelBin->gfx_list_offset);
    modelRenderTextureList = modelRenderTextureList ? modelRenderTextureList : (BKTextureList *)((uintptr_t)modelRenderModelBin + (uintptr_t)(u32)modelRenderModelBin->texture_list_offset);
    modelRendervertexList = modelRendervertexList ? modelRendervertexList : (BKVertexList *)((uintptr_t)modelRenderModelBin + (uintptr_t)(u32)modelRenderModelBin->vtx_list_offset);
    modelRenderCameraAreaList = (modelRenderModelBin->camera_area_list_offset == 0) ? NULL : (BKCameraAreaList *)((uintptr_t)model_bin + (uintptr_t)(u32)model_bin->camera_area_list_offset);

    if (D_80383710) {
        tmp_f0 = D_80383708 - 500.0f;
        if(tmp_f0 < camera_focus_distance){
            alpha = (s32)((1.0f - (camera_focus_distance - tmp_f0)/500.0f)*255.0f);
            // [port] Preserve model fading with Disable Culling
            // No-cull can keep a model alive beyond the original final cutoff.
            // Clamp the completed fade instead of allowing alpha to wrap.
            if (alpha < 0) alpha = 0;
            else if (alpha > 0xFF) alpha = 0xFF;
            EventSystem_Should(VB_MODEL_DRAWDIST_FADE_ALPHA, true, &alpha);
            if(modelRenderColorMode == COLOR_MODE_DYNAMIC_PRIM_AND_ENV){
                modelRenderDynColors.prim[3] = (modelRenderDynColors.prim[3] * alpha) / 0xff;
            }
            else if(modelRenderColorMode == COLOR_MODE_DYNAMIC_ENV){
                modelRenderDynEnvColor[3] = (modelRenderDynEnvColor[3] * alpha)/0xff;
            }
            else if(modelRenderColorMode == COLOR_MODE_STATIC_OPAQUE){
                modelRender_setAlpha(alpha);
            }
            else if(modelRenderColorMode == COLOR_MODE_STATIC_TRANSPARENT){
                modelRenderDynAlpha = (modelRenderDynAlpha *alpha)/0xff;
            }
        }
    }

    // Set up segments 1 and 2 to point to vertices and textures respectively
    gSPSegment((*gfx)++, 0x01, osVirtualToPhysical((void *)&modelRendervertexList->vertices));
    uintptr_t base_tex = (uintptr_t)&modelRenderTextureList->texture_infos[modelRenderTextureList->count];
    gSPSegment((*gfx)++, 0x02, osVirtualToPhysical((void *)base_tex));


    //segments 11 to 15 contain animated textures
    if(modelRenderAnimatedTexturesCacheId){
        int i_segment;
        s32 texture_offset;

        for(i_segment = 0; i_segment < 4; i_segment++){
            if(AnimTextureListCache_tryGetTextureOffset(modelRenderAnimatedTexturesCacheId, i_segment, &texture_offset)){
                uintptr_t base = (uintptr_t)&modelRenderTextureList->texture_infos[modelRenderTextureList->count];
                gSPSegment((*gfx)++, 15 - i_segment, osVirtualToPhysical((void *)(base + (uintptr_t)texture_offset)));
            }
        }
    }

    if(modelRenderDepthMode != MODEL_RENDER_DEPTH_NONE){
        gSPSetGeometryMode((*gfx)++, G_ZBUFFER);
    }
    else{
        gSPClearGeometryMode((*gfx)++, G_ZBUFFER);
    }

    // Pick a table of render modes for opaque and translucent rendering
    if(modelRenderDepthMode == MODEL_RENDER_DEPTH_NONE){ // No depth buffering
        rendermode_table_opa = renderModesNoDepthOpa;
        rendermode_table_xlu = renderModesNoDepthXlu;
    }
    else if(modelRenderDepthMode == MODEL_RENDER_DEPTH_FULL){ // Full depth buffering
        rendermode_table_opa = renderModesFullDepthOpa;
        rendermode_table_xlu = renderModesFullDepthXlu;
    }
    else if(modelRenderDepthMode == MODEL_RENDER_DEPTH_COMPARE){ // Depth compare but no depth write
        rendermode_table_opa = renderModesDepthCompareOpa;
        rendermode_table_xlu = renderModesDepthCompareXlu;
    }

    if(modelRenderColorMode == COLOR_MODE_DYNAMIC_PRIM_AND_ENV){
        s32 alpha;
        
        alpha = modelRenderDynColors.prim[3] + (modelRenderDynColors.env[3]*(0xFF - modelRenderDynColors.prim[3]))/0xff;
        gSPDisplayList((*gfx)++, setup2CycleDL);
        gDPSetEnvColor((*gfx)++, modelRenderDynColors.env[0], modelRenderDynColors.env[1], modelRenderDynColors.env[2], alpha);
        gDPSetPrimColor((*gfx)++, 0, 0, modelRenderDynColors.prim[0], modelRenderDynColors.prim[1], modelRenderDynColors.prim[2], 0);

        // Set up segment 3 to point to the right render mode table based on the alpha value
        if(alpha == 0xFF){
            gSPSegment((*gfx)++, 0x03, osVirtualToPhysical(rendermode_table_opa));
        }
        else{
            gSPSegment((*gfx)++, 0x03, osVirtualToPhysical(rendermode_table_xlu));
        }
        //TODO
    }
    else if(modelRenderColorMode == COLOR_MODE_DYNAMIC_ENV){
        gSPDisplayList((*gfx)++, setup2CycleBlackPrimDL);
        gDPSetEnvColor((*gfx)++, modelRenderDynEnvColor[0], modelRenderDynEnvColor[1], modelRenderDynEnvColor[2], modelRenderDynEnvColor[3]);

        // Set up segment 3 to point to the right render mode table based on the alpha value
        if(modelRenderDynEnvColor[3] == 0xFF){
            gSPSegment((*gfx)++, 0x03, osVirtualToPhysical(rendermode_table_opa));
        }
        else{
            gSPSegment((*gfx)++, 0x03, osVirtualToPhysical(rendermode_table_xlu));
        }
    }
    else if(modelRenderColorMode == COLOR_MODE_STATIC_OPAQUE){
        gSPDisplayList((*gfx)++, setup2CycleWhiteEnvDL);
        gSPSegment((*gfx)++, 0x03, osVirtualToPhysical(rendermode_table_opa));
    }
    else if(modelRenderColorMode == COLOR_MODE_STATIC_TRANSPARENT){
        gSPDisplayList((*gfx)++, setup2CycleDL_copy);
        gDPSetEnvColor((*gfx)++, 0xFF, 0xFF, 0xFF, modelRenderDynAlpha);
        gSPSegment((*gfx)++, 0x03, osVirtualToPhysical(rendermode_table_xlu));
    }

    if(modelRenderModelBin->geo_type & 2){ //trilinear mipmapping
        gSPDisplayList((*gfx)++, mipMapWrapDL);
    }

    if(modelRenderModelBin->geo_type & 4){ //env mapping
        if(0.0f == camera_focus[2]){
            camera_focus[2] = -0.1f;
        }
        guLookAtReflect(*mtx, D_803837E0.cur_lookat,
            D_803837E0.eye_pos[0], D_803837E0.eye_pos[1], D_803837E0.eye_pos[2],
            camera_focus[0], camera_focus[1], camera_focus[2],
            0.0f, 1.0f, 0.0f);
        gSPLookAt((*gfx)++, D_803837E0.cur_lookat);
        osWritebackDCache(D_803837E0.cur_lookat, sizeof(LookAt));
        D_803837E0.cur_lookat++;
        if(D_803837E0.cur_lookat == D_803837E0.lookat_buffer_end)
            D_803837E0.cur_lookat = D_803837E0.lookat_buffer;
    }

    if(D_8038371C && !modelRenderModelBin->animation_list_offset){
        D_8038371C = 0;
    }
    else if(D_8038371C == 0 && modelRenderModelBin->animation_list_offset){
        if(modelRenderBoneTransformList == NULL){
            animMtxList_setBoneless(&modelRenderAnimMtxList, (BKAnimationList*)((u8*)model_bin + (uintptr_t)(u32)model_bin->animation_list_offset));
        }
        else{
            animMtxList_setBoned(&modelRenderAnimMtxList, (BKAnimationList*)((u8*)model_bin + (uintptr_t)(u32)model_bin->animation_list_offset), modelRenderBoneTransformList);
        }
        D_8038371C = modelRenderAnimMtxList;
    }

    if(modelRenderCameraAreaList){
        cameraAreaList_updateInBoundsFlag(modelRenderCameraAreaList, modelRenderCameraPosition, scale);
    }

    if(model_bin->anim_vertices_list_offset != 0 && D_8038371C != NULL){
        animVerticesList_transform((BKAnimVerticesList *)((uintptr_t)modelRenderModelBin + (uintptr_t)(u32)modelRenderModelBin->anim_vertices_list_offset), modelRendervertexList, D_8038371C);
        // [port] The transform just posed the model's shared vertex array. Hand this draw a private copy.
        port_modelRender_snapshotAnimVertices(gfx, vtxList_getVertices(modelRendervertexList), vtxList_getVtxCount(modelRendervertexList));
    }

    mlMtxIdent();
    if(D_80383758.unk18){
        func_80252AF0(D_80383758.unk1C, object_position, rotation, scale, arg5);
    }
    else{
        func_80252AF0(modelRenderCameraPosition, object_position, rotation, scale, arg5);
    }

    if(D_803837B0.unk0){
        mlMtxRotatePYR(D_803837B0.unk4[0], D_803837B0.unk4[1], D_803837B0.unk4[2]);
    }
    // [port] Mirror mode: bake counter-mirror into modelview for text-bearing models
    int _mirror_excluded = port_mirror_bakeCounterScale();
    mlMtxGet(&D_80383BF8);

    mlMtxApply(*mtx);
    gSPMatrix((*gfx)++, (*mtx)++, G_MTX_PUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    
    modelRenderScale = scale;
    if(rotation){
        modelRenderRotation[0] = rotation[0];
        modelRenderRotation[1] = rotation[1];
        modelRenderRotation[2] = rotation[2];
    }
    else{
        modelRenderRotation[0] = modelRenderRotation[1] = modelRenderRotation[2] = 0.0f;
    }


    // [port] Mirror mode: counter-mirror text-bearing models so text reads correctly
    if (_mirror_excluded) gSPClearExtraGeometryMode((*gfx)++, G_EX_INVERT_CULLING);
    if (port_shouldDisableCulling()) {
        lighthouse_setFrustumChecksEnabled(false);
    }
    modelRender_executeGeoCmds(gfx, mtx, (BKGeoCmd *)((u8 *)model_bin + model_bin->geo_list_offset));
    if (port_shouldDisableCulling()) {
        lighthouse_setFrustumChecksEnabled(true);
    }
    // [port] Mirror mode: restore culling inversion
    if (_mirror_excluded) gSPSetExtraGeometryMode((*gfx)++, G_EX_INVERT_CULLING);
    gSPPopMatrix((*gfx)++, G_MTX_MODELVIEW);
    // [port] Romhack model DLs can leave a palette (TLUT) mode enabled and leak it
    // into the next model's draw; reset it after every model for romhacks.
    port_modelRenderResetTLUT(gfx);

    if(modelRenderCallback.post_draw != NULL){
        modelRenderCallback.post_draw(modelRenderCallback.post_draw_arg);
    }

    if(sSecondaryModelData.model_id){
        assetCache_free(model_bin);
    }


    modelRender_reset();
    if (port_shouldDisableCulling()) {
        if (port_isDemoPlayback()) {
            // Preserve the original visibility result after the visual-only draw.
            D_80370990 = !cur_model_would_have_been_culled_in_demo;
        } else {
            D_80370990 = true;
        }
        cur_model_would_have_been_culled_in_demo = false;
    }
    return model_bin;
}

BKAnimVerticesList *modelbin_getAnimVerticesList(BKModelBin *arg0){
    if(arg0->anim_vertices_list_offset == 0)
        return NULL;
    return (BKAnimVerticesList *)((u8*)arg0 + arg0->anim_vertices_list_offset);
}

BoneTransformList *modelRender_getBoneTransformList(void){
    return modelRenderBoneTransformList;
}

BkGeoType modelbin_getGeoType(BKModelBin *arg0){
    return arg0->geo_type;
}

BKGfxList *modelbin_getGfxList(BKModelBin *arg0){
    return (BKGfxList *)((u8*)arg0 + arg0->gfx_list_offset);
}

BKCollisionList *modelbin_getCollisionList(BKModelBin *arg0){
    if(arg0 == NULL)
        return NULL;
    
    if(arg0->collision_list_offset == 0)
        return NULL;

    return (BKCollisionList *)((u8*)arg0 + arg0->collision_list_offset);
}

BKMeshList *modelbin_getMeshList(BKModelBin *arg0){
    if(arg0->mesh_list_offset == 0)
        return NULL;

    return (BKMeshList *)((u8*)arg0 + arg0->mesh_list_offset);
}

f32 modelbin_getUnk34(BKModelBin *arg0){
    return *(f32 *)((u8*)arg0 + 0x34);
}

BKAnimationList *modelbin_getAnimationList(BKModelBin *arg0){
    if(arg0->animation_list_offset == 0)
        return NULL;

    return (BKAnimationList *)((u8*)arg0 + arg0->animation_list_offset);
}

s32 modelRender_func_8033A0F0(s32 arg0){
    return D_80383658[arg0];
}

BKTextureList *modelbin_getTextureList(BKModelBin *model_bin){
    return (BKTextureList *)((u8*)model_bin + model_bin->texture_list_offset);
}

BKAnimTextureList *modelbin_getAnimTextureList(BKModelBin *model_bin){
    if(model_bin->animated_texture_list_offset == 0)
        return NULL;
    return (BKAnimTextureList *)((u8*)model_bin + model_bin->animated_texture_list_offset);
}

BKModelUnk14List *modelbin_getUnk14List(BKModelBin *this){
    if(this->unk14_list_offset == 0)
        return 0;
    return (BKModelUnk14List *)((u8*)this + this->unk14_list_offset);
}

BKVertexList *modelbin_getVtxList(BKModelBin *arg0){
    return (BKVertexList *)((u8*)arg0 + arg0->vtx_list_offset);
}

BKCameraAreaList *modelbin_getCameraAreaList(BKModelBin *arg0){
    return (arg0->camera_area_list_offset == 0) ? NULL : (BKCameraAreaList *)((u8*)arg0 + arg0->camera_area_list_offset);
}

bool modelRender_func_8033A170(void){
    return D_80370990;
}

void modelRender_free(void){
    animMtxList_free(modelRenderAnimMtxList);
    modelRenderAnimMtxList = NULL;
}

void modelRender_init(void){
    modelRender_reset();
    D_80383758.unk18 = 0;
    D_803837E0.cur_lookat = D_803837E0.lookat_buffer;
    D_803837E0.lookat_buffer_end = D_803837E0.cur_lookat + ARRAYLEN(D_803837E0.lookat_buffer);
    D_803837E0.eye_pos[0] = D_803837E0.eye_pos[1] = D_803837E0.eye_pos[2] = 0.0f;
    modelRenderAnimMtxList = animMtxList_new();
}

void modelRender_func_8033A1FC(void){
    s32 i;
    for(i = 0; i < 0x2A; i++){
        D_80383658[i] = 0;
    }
}

void modelRender_setBoneTransformList(BoneTransformList *arg0){
    modelRenderBoneTransformList = arg0;
}

f32 modelRender_func_8033A244(f32 arg0){
    f32 out = D_80383708;
    D_80383708 = arg0;
    return out;
}

void modelRender_func_8033A25C(bool arg0){
    D_80383704 = BOOL(arg0);
}  

void modelRender_func_8033A280(f32 arg0){
    D_8038370C = arg0;
}

void modelRender_func_8033A28C(bool arg0){
    D_80383710 = arg0;
}

void modelRender_func_8033A298(bool arg0){
    D_80383758.unk18 = arg0;
    if(arg0){
        viewport_getPosition_vec3f(D_80383758.unk0);
        viewport_getRotation_vec3f(D_80383758.unkC);
    }
}

/* moderRender_preDraw() sets a generic 1 argument function that will
 * be called immediately prior to the model being drawn
 */
void modelRender_setPreDrawCallback(model_render_pre_draw_callback_f func, void *arg){
    modelRenderCallback.pre_draw = func;
    modelRenderCallback.pre_draw_arg = arg;
}

/* moderRender_postDraw() sets a generic 1 argument function that will
 * be called immediately after to the model has been drawn
 */
void modelRender_setPostDrawCallback(model_render_post_draw_callback_f func, void *arg){
    modelRenderCallback.post_draw = func;
    modelRenderCallback.post_draw_arg = arg;
}

void modelRender_setDisplayList(BKGfxList *gfx_list){
    modelRenderDisplayList = gfx_list;
}

void modelRender_func_8033A308(f32 arg0[3]){
    D_803837B0.unk0 = true;
    D_803837B0.unk4[0] = arg0[0];
    D_803837B0.unk4[1] = arg0[1];
    D_803837B0.unk4[2] = arg0[2];
}

void modelRender_setPrimAndEnvColors(s32 env[4], s32 prim[4]){
    modelRenderColorMode = COLOR_MODE_DYNAMIC_PRIM_AND_ENV;

    modelRenderDynColors.env[0] = env[0];
    modelRenderDynColors.env[1] = env[1];
    modelRenderDynColors.env[2] = env[2];
    modelRenderDynColors.env[3] = env[3];

    modelRenderDynColors.prim[0] = prim[0];
    modelRenderDynColors.prim[1] = prim[1];
    modelRenderDynColors.prim[2] = prim[2];
    modelRenderDynColors.prim[3] = prim[3];
}

void modelRender_setEnvColor(s32 r, s32 g, s32 b, s32 a){
    modelRenderColorMode = COLOR_MODE_DYNAMIC_ENV;

    modelRenderDynEnvColor[0] = MIN(0xFF, r);
    modelRenderDynEnvColor[1] = MIN(0xFF, g);
    modelRenderDynEnvColor[2] = MIN(0xFF, b);
    modelRenderDynEnvColor[3] = MIN(0xFF, a);
}

void modelRender_setAlpha(s32 a){
    if(a == 0xff){
        modelRenderColorMode = COLOR_MODE_STATIC_OPAQUE;
    }
    else{
        modelRenderColorMode = COLOR_MODE_STATIC_TRANSPARENT;
        modelRenderDynAlpha = a;
    }
}

void modelRender_func_8033A444(AnimMtxList *arg0){
    D_8038371C = arg0;
}

void modelRender_setRefPoints(Vec3fArray *arg0){
    modelRenderRefPoints = arg0;
}

void modelRender_setAppendageVisibility(s32 appendage_id, s32 appendage_visibility){
    D_80383658[appendage_id] = appendage_visibility;
}

void modelRender_func_8033A470(s32 arg0, s32 arg1){
    D_80383658[arg0] = -arg1;
}

void modelRender_setTextureList(BKTextureList *textureList){
    modelRenderTextureList = textureList;
}

void modelRender_setAnimatedTexturesCacheId(s32 arg0){
    modelRenderAnimatedTexturesCacheId = arg0;
}

void modelRender_setSecondaryModel(enum asset_e modelId, f32 arg1, f32 arg2){
    sSecondaryModelData.model_id = modelId;
    sSecondaryModelData.unk4 = arg1;
    sSecondaryModelData.unk8 = arg2;
}

void modelRender_setVertexList(BKVertexList *vertexList){
    modelRendervertexList = vertexList;
}

void modelRender_setDepthMode(enum model_render_depth_mode_e renderMode){
    modelRenderDepthMode = renderMode;
}

void modelRender_defrag(void){
    if(modelRenderAnimMtxList != NULL){
        modelRenderAnimMtxList = animMtxList_defrag(modelRenderAnimMtxList);
    }
}
