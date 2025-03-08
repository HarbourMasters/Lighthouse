#include "functions.h"
#include "variables.h"
#include <ultra64.h>

//text.c

typedef struct {
  void *unk0;
  u8 pad4[0x4];
  s32 *unk8;
  u8 padC[0x4];
} TextRendererFont;

typedef struct {
  BKSprite_s *font_bin;
  u8 font_id;
  // u8 pad5[0x3];
  BKSpriteTextureBlock **letter_texture;
  u8 half_width;
  u8 height;
  u8 padE[0x2];
} TextRendererFontInfo;

typedef struct {
  s32 unk0;
  s32 unk4;
  s32 unk8;
  u16 unkC;
  u8 unkE;
  u8 unkF[3];
  u8 unk12[3];
  u8 pad15[0x3];
  s32 unk18;
} TextRendererCharacter;

typedef struct {
  TextRendererCharacter *unk0;
  TextRendererFontInfo *unk4;
  char *string;
  s32 unkC;
  s32 unk10;
  s32 string_len;
  u32 flags;
  u8 unk1C[3];
  u8 pad1F[1];
} TextRendererSruct;

u8 defaultColor1[3] = {0xFF, 0x8B, 0};
u8 defaultColor2[3] = {0xFF, 0x8B, 0};
u8 defaultColor3[3] = {0xFF, 0x8B, 0};
u8 defaultColor4[3] = {0xFF, 0x8B, 0};
u8 highlightColor1[3] = {0xFF, 0xFF, 0XFF};
u8 highlightColor2[3] = {0xFF, 0x00, 0X46};
u8 defaultColor5[3] = {0xFF, 0x8B, 0};

/* .bss */
TextRendererSruct *textRenderer;

/* public */
void defragTextRenderer(s32 arg0);

/* .code */
TextRendererSruct *initTextRenderer(void) {
  #ifndef LIGHTHOUSE_P
  u8 sp24[3] = defaultColor1;
  #else
  u8 sp24[3];
  sp24[0] = defaultColor1[0];
  sp24[1] = defaultColor1[1];
  sp24[2] = defaultColor1[2];
  #endif
  textRenderer = (TextRendererSruct *)heap_malloc(sizeof(TextRendererSruct));
  textRenderer->unk0 = (TextRendererCharacter *)heap_malloc(sizeof(TextRendererCharacter));
  textRenderer->unkC = 0;
  textRenderer->unk4 = (TextRendererFontInfo *)heap_malloc(sizeof(TextRendererFontInfo));
  textRenderer->unk10 = 0;
  textRenderer->string = (char *)heap_malloc(sizeof(char));
  textRenderer->string_len = 0;
  textRenderer->flags = 0;
  textRenderer->unk1C[0] = sp24[0];
  textRenderer->unk1C[1] = sp24[1];
  textRenderer->unk1C[2] = sp24[2];
  return textRenderer;
}

void freeFontTextures(TextRendererFontInfo *arg0, u32 arg1) {
  while (arg1--) {
    assetcache_release(arg0[arg1].font_bin);
    bk_free(arg0[arg1].letter_texture);
  }
  bk_free(arg0);
}

void freeTextRenderer(void) {
  if (textRenderer->unk0 != NULL) {
    bk_free(textRenderer->unk0);
  }
  if (textRenderer->unk4 != NULL) {
    freeFontTextures(textRenderer->unk4, textRenderer->unk10);
  }
  if (textRenderer->string != NULL) {
    bk_free(textRenderer->string);
  }
  bk_free(textRenderer);
  textRenderer = NULL;
}

s32 findFontIndex(u8 arg0) {
  u32 var_v1;
  var_v1 = textRenderer->unk10;
  while (var_v1--) {
    if (arg0 == textRenderer->unk4[var_v1].font_id) {
      return var_v1;
    }
  }
  return -1;
}

BKSpriteTextureBlock *getFontTexture(s32 arg0, char arg1) {
  return textRenderer->unk4[arg0].letter_texture[arg1 - 0x21];
}

/* font_bin to char texture blocks */
BKSpriteTextureBlock **loadFontTextures(BKSprite_s *sprite) {
  BKSpriteFrame *frame;
  BKSpriteTextureBlock **chunkPtrArray;
  BKSpriteTextureBlock *chunk;
  s32 var_v1;
  s32 i;
  s32 chunk_size;

  frame = sprite_getFramePtr(sprite, 0);
  chunkPtrArray = (BKSpriteTextureBlock **)heap_malloc((frame->chunkCnt + 1) * 4);
  chunk = (BKSpriteTextureBlock *)(frame + 1);
  for (i = 0; i < frame->chunkCnt; i++) {
    chunkPtrArray[i] = chunk;
    chunk_size = (chunk->w * chunk->h);
    var_v1 = (s32)(chunk + 1);
    while ((var_v1 % 8)) {
      var_v1++;
    }
    chunk = (BKSpriteTextureBlock *)(var_v1 + chunk_size);
  }
  return chunkPtrArray;
}

/* get index  from font_id */
s32 loadFont(u8 font_id) {
  s32 sp24;

  sp24 = findFontIndex(font_id);
  if (sp24 == -1) { // font not loaded
    sp24 = textRenderer->unk10++;
    if (textRenderer->unk10 > 1) {
      textRenderer->unk4 = (TextRendererFontInfo *)bk_realloc(
          textRenderer->unk4, (textRenderer->unk10 + 1) * sizeof(TextRendererFontInfo));
    }
    textRenderer->unk4[sp24].font_id = font_id;
    textRenderer->unk4[sp24].font_bin =
        (BKSprite_s *)assetcache_get(font_id + 0x6E9);
    textRenderer->unk4[sp24].letter_texture =
        loadFontTextures(textRenderer->unk4[sp24].font_bin);
    textRenderer->unk4[sp24].half_width =
        textRenderer->unk4[sp24].letter_texture['W' - 0x21]->x / 2;
    textRenderer->unk4[sp24].height =
        textRenderer->unk4[sp24].letter_texture['W' - 0x21]->y;
  }
  defragTextRenderer(5);
  return sp24;
}

s32 appendString(char *arg0) {
  s32 sp1C;
  s32 temp_a2;

  sp1C = textRenderer->string_len;
  textRenderer->string_len += bk_strlen(arg0) + 1;
  if (textRenderer->string_len >= 2) {
    textRenderer->string =
        (char *)bk_realloc(textRenderer->string, textRenderer->string_len + 1);
  }
  bk_strcpy(textRenderer->string + sp1C, arg0);
  return sp1C;
}

void addText(s32 arg0, s32 arg1, s32 arg2, char *arg3, u8 rgb[3]) {
  s32 sp24;

  sp24 = textRenderer->unkC++;
  if (textRenderer->unkC >= 2) {
    textRenderer->unk0 =
        bk_realloc(textRenderer->unk0, (textRenderer->unkC * 0x1C) + 0x1C);
  }
  textRenderer->unk0[sp24].unk0 = arg0;
  textRenderer->unk0[sp24].unk4 = arg1;
  textRenderer->unk0[sp24].unkE = arg2;
  textRenderer->unk0[sp24].unkC = appendString(arg3);
  textRenderer->unk0[sp24].unk8 = textRenderer->flags;
  textRenderer->unk0[sp24].unkF[0] = rgb[0];
  textRenderer->unk0[sp24].unkF[1] = rgb[1];
  textRenderer->unk0[sp24].unkF[2] = rgb[2];
  textRenderer->unk0[sp24].unk18 = -1;
}

void resetTextRenderer(void) {
  textRenderer->unkC = 0;
  textRenderer->string_len = 0;
}

s32 calculateTextWidth(char *str, s32 arg1, s32 start, u32 flags) {
  s32 tab_width;
  s32 position;
  char *var_s1;
  s32 i;

  if (flags & 2) {
    return 2 * textRenderer->unk4[arg1].half_width * bk_strlen(str);
  }

  position = start;
  for (var_s1 = str; *var_s1; var_s1++) {
    if (*var_s1 == ' ') {
      position += textRenderer->unk4[arg1].half_width;
    } else if (*var_s1 == '\t') {
      position++;
      tab_width = 4 * textRenderer->unk4[arg1].half_width;
      while (position % tab_width) {
        position++;
      };
    } else {
      position += getFontTexture(arg1, *var_s1)->x;
    }
  }
  return position - start;
}

void renderCharacter(TextRendererCharacter *arg0, char arg1, s32 *arg2, s32 *arg3,
                     Gfx **gfx) {
  s32 timg;
  BKSpriteTextureBlock *chunk;

  if (arg1 == ' ') {
    *arg2 += textRenderer->unk4[arg0->unkE].half_width;
  } else if (arg1 == '\t') {
    (*arg2)++;
    while ((*arg2 % (s32)(textRenderer->unk4[arg0->unkE].half_width * 4)) != 0) {
      (*arg2)++;
    }
  } else {
    chunk = getFontTexture(arg0->unkE, arg1);
    timg = (s32)(chunk + 1);
    while ((timg % 8)) {
      timg++;
    }

    gDPSetTextureImage((*gfx)++, G_IM_FMT_I, G_IM_SIZ_8b, chunk->w, timg);
    gDPSetTile((*gfx)++, G_IM_FMT_I, G_IM_SIZ_8b,
               (((((chunk->w - 0) + 1) * 1) + 7) >> 3), 0x0000, G_TX_LOADTILE,
               0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
    gDPLoadSync((*gfx)++);
    gDPLoadTile((*gfx)++, G_TX_LOADTILE, 0, 0, (chunk->w << 2),
                (chunk->h << 2));
    gDPPipeSync((*gfx)++);
    gDPSetTile((*gfx)++, G_IM_FMT_I, G_IM_SIZ_8b,
               (((((chunk->w - 0) + 1) * 1) + 7) >> 3), 0x0000, G_TX_RENDERTILE,
               0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
    gDPSetTileSize((*gfx)++, G_TX_RENDERTILE, 0, 0, chunk->w << 2,
                   chunk->h << 2);
    gSPScisTextureRectangle(
        (*gfx)++, (*arg2 << 2), (*arg3 << 2), ((*arg2 + chunk->w) << 2),
        ((*arg3 + chunk->h) << 2), G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400);
    if (arg0->unk8 & 2) {
      *arg2 += textRenderer->unk4[arg0->unkE].half_width * 2;
    } else {
      *arg2 += chunk->x;
    }
  }
}

//clang-format off
void renderText(TextRendererCharacter *arg0, Gfx **gfx) {
    s32 temp_v0;
    s32 spA8;
    s32 spA4;
    s32 var_s3;
    char var_s2;
    u32 var_a3;

    var_a3 = arg0->unk8;
    if (!(var_a3 & 4)) { //draw transparent grey background
        temp_v0 = calculateTextWidth(&textRenderer->string[arg0->unkC], arg0->unkE, arg0->unk0, var_a3);
        gDPPipeSync((*gfx)++);
        gDPSetPrimColor((*gfx)++, 0, 0, 0x28, 0x28, 0x28, 0x96);
        gDPSetCombineMode((*gfx)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPScisFillRectangle((*gfx)++, 
            (arg0->unk0 - 2),
            arg0->unk4,
            (arg0->unk0 + temp_v0 + 2),
            (textRenderer->unk4[arg0->unkE].height + arg0->unk4)
        );
        gDPPipeSync((*gfx)++);
    }
    gDPSetCombineLERP((*gfx)++, PRIMITIVE, ENVIRONMENT, TEXEL0_ALPHA, ENVIRONMENT, 0, 0, 0, TEXEL0, PRIMITIVE, ENVIRONMENT, TEXEL0_ALPHA, ENVIRONMENT, 0, 0, 0, TEXEL0);

    if (arg0->unk18 == -1) {
        gDPSetPrimColor((*gfx)++, 0, 0, arg0->unkF[0], arg0->unkF[1], arg0->unkF[2], 0);
        gDPSetEnvColor((*gfx)++, arg0->unkF[0], arg0->unkF[1], arg0->unkF[2], 0);
        spA8 = arg0->unk0;
        spA4 = arg0->unk4;
        for(var_s3 = 0; (var_s2 = textRenderer->string[var_s3 + arg0->unkC]) != 0; var_s3++) {
            renderCharacter(arg0, var_s2, &spA8, &spA4, gfx);
        }
    } else {
        spA8 = arg0->unk0;
        spA4 = arg0->unk4;
        for(var_s3 = 0; (var_s2 = textRenderer->string[var_s3 + arg0->unkC]); var_s3++) {
            if (var_s3 == arg0->unk18) {
                gDPSetPrimColor((*gfx)++, 0, 0, arg0->unk12[0], arg0->unk12[1], arg0->unk12[2], 0x00);
                gDPSetEnvColor((*gfx)++, arg0->unk12[0], arg0->unk12[1], arg0->unk12[2], 0);
                if (var_s2 == ' ') {
                    s32 sp6C, sp68;
                    s32 sp64, sp60;
                    sp64 = spA4 - 5;\
                    sp60 = spA4 + 5;
                    sp68 = sp6C = spA8;
                    renderCharacter(arg0, '-', &sp6C, &sp64, gfx);
                    renderCharacter(arg0, '-', &sp68, &sp60, gfx);
                }
            } else {
                gDPSetPrimColor((*gfx)++, 0, 0, arg0->unkF[0], arg0->unkF[1], arg0->unkF[2], 0);
                gDPSetEnvColor((*gfx)++, arg0->unkF[0], arg0->unkF[1], arg0->unkF[2], 0);
            }
            renderCharacter(arg0, var_s2, &spA8, &spA4, gfx);
        }
    }
}
//clang-format on

void prepareTextRendering(Gfx **gdl) {
  int i;

  gDPPipeSync((*gdl)++);
  gSPClearGeometryMode((*gdl)++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG |
                                     G_LIGHTING | G_TEXTURE_GEN |
                                     G_TEXTURE_GEN_LINEAR | G_LOD |
                                     G_SHADING_SMOOTH);
  gSPTexture((*gdl)++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
  gSPSetGeometryMode((*gdl)++, G_TEXTURE_GEN_LINEAR);
  gDPSetCycleType((*gdl)++, G_CYC_1CYCLE);
  gDPPipelineMode((*gdl)++, G_PM_NPRIMITIVE);
  gDPSetTextureLOD((*gdl)++, G_TL_TILE);
  gDPSetTextureLUT((*gdl)++, G_TT_NONE);
  gDPSetTextureDetail((*gdl)++, G_TD_CLAMP);
  gDPSetTexturePersp((*gdl)++, G_TP_NONE);
  gDPSetTextureFilter((*gdl)++, G_TF_BILERP);
  gDPSetTextureConvert((*gdl)++, G_TC_FILT);
  gDPSetAlphaCompare((*gdl)++, G_AC_NONE);
  gDPSetRenderMode((*gdl)++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
  for (i = 0; i < textRenderer->unkC; i++) {
    textRenderer->unk0[i].unk4 -= 0x10;
    renderText(textRenderer->unk0 + i, gdl);
    textRenderer->unk0[i].unk4 += 0x10;
  }
  gDPSetTexturePersp((*gdl)++, G_TP_PERSP);
}

void renderAndResetText(Gfx **gdl) {
  prepareTextRendering(gdl);
  resetTextRenderer();
}

void initializeTextRenderer(void) {
  textRenderer = initTextRenderer();
  loadFont(0);
}

void cleanupTextRenderer(void) { freeTextRenderer(); }

void addTextWithDefaultColor1(s32 arg0, s32 arg1, char *arg2) {
  s32 sp24;
  #ifndef LIGHTHOUSE_P
  u8 sp20[3] = defaultColor2;
  #else
  u8 sp20[3];
  sp20[0] = defaultColor2[0];
  sp20[1] = defaultColor2[1];
  sp20[2] = defaultColor2[2];
  #endif

  sp24 = loadFont(0);
  addText(arg0, arg1, sp24, arg2, sp20);
}

void addTextWithDefaultColor2(s32 arg0, s32 arg1, char *arg2) {
  s32 sp24;
  #ifndef LIGHTHOUSE_P
  u8 sp20[3] = defaultColor3;
  #else
  u8 sp20[3];
  sp20[0] = defaultColor3[0];
  sp20[1] = defaultColor3[1];
  sp20[2] = defaultColor3[2];
  #endif
  sp24 = loadFont(0);
  addText(arg0, arg1, sp24, arg2, sp20);
}

void addTextWithDefaultColor3(s32 arg0, s32 arg1, char *arg2) {
  s32 sp2C;
  #ifndef LIGHTHOUSE_P
  u8 sp28[3] = defaultColor4;
  #else
  u8 sp28[3];
  sp28[0] = defaultColor4[0];
  sp28[1] = defaultColor4[1];
  sp28[2] = defaultColor4[2];
  #endif

  sp2C = loadFont(0);
  arg0 *= textRenderer->unk4[sp2C].half_width;
  arg1 *= textRenderer->unk4[sp2C].height;
  addText(arg0, arg1, sp2C, arg2, sp28);
}

void addColoredText(s32 arg0, s32 arg1, s32 arg2, char *arg3, u8 arg4[3],
                    u8 arg5[3], s32 arg6) {
  s32 sp24;

  sp24 = textRenderer->unkC++;
  if (textRenderer->unkC >= 2) {
    textRenderer->unk0 = (TextRendererCharacter *)bk_realloc(
        textRenderer->unk0, (textRenderer->unkC + 1) * sizeof(TextRendererCharacter));
  }
  textRenderer->unk0[sp24].unk0 = arg0;
  textRenderer->unk0[sp24].unk4 = arg1;
  textRenderer->unk0[sp24].unkE = arg2;
  textRenderer->unk0[sp24].unkC = appendString(arg3);
  textRenderer->unk0[sp24].unk8 = textRenderer->flags;

  textRenderer->unk0[sp24].unkF[0] = arg4[0];
  textRenderer->unk0[sp24].unkF[1] = arg4[1];
  textRenderer->unk0[sp24].unkF[2] = arg4[2];

  textRenderer->unk0[sp24].unk12[0] = arg5[0];
  textRenderer->unk0[sp24].unk12[1] = arg5[1];
  textRenderer->unk0[sp24].unk12[2] = arg5[2];

  textRenderer->unk0[sp24].unk18 = arg6;
}

void addColoredTextWithHighlight(s32 arg0, s32 arg1, char *arg2, s32 arg3) {
  s32 temp_v0;
  #ifndef LIGHTHOUSE_P
  u8 sp30[3] = highlightColor1;
  u8 sp2C[3] = highlightColor2;
  #else
  u8 sp30[3];
  sp30[0] = highlightColor1[0];
  sp30[1] = highlightColor1[1];
  sp30[2] = highlightColor1[2];
  
  u8 sp2C[3];
  sp2C[0] = highlightColor2[0];
  sp2C[1] = highlightColor2[1];
  sp2C[2] = highlightColor2[2];
  #endif

  temp_v0 = loadFont(0);
  arg0 *= textRenderer->unk4[temp_v0].half_width;
  arg1 *= textRenderer->unk4[temp_v0].height;
  addColoredText(arg0, arg1, temp_v0, arg2, sp30, sp2C, arg3);
}

void setTextColor(u8 arg0, u8 arg1, u8 arg2) {
  textRenderer->unk1C[0] = arg0;
  textRenderer->unk1C[1] = arg1;
  textRenderer->unk1C[2] = arg2;
}

void setTextColorFrom16Bit(u16 arg0) {
  textRenderer->unk1C[0] = (u8)(arg0 >> 0xb) << 3;
  textRenderer->unk1C[1] = (u8)(arg0 >> 0x1) << 3;
  textRenderer->unk1C[2] = (u8)(arg0 >> 0x6) << 3;
}

u8 scaleColorComponent(u8 arg0, f32 arg1) {
  s32 var_v1;

  var_v1 = (s32)((f32)(s32)arg0 * arg1);
  var_v1 = (var_v1 > 0xFF) ? 0xff : var_v1;
  return var_v1;
}

void scaleTextColor(u8 arg0, u8 arg1, u8 arg2, f32 arg3) {
  setTextColor(scaleColorComponent(arg0, arg3), scaleColorComponent(arg1, arg3),
               scaleColorComponent(arg2, arg3));
}

u16 setTextColorFrom8Bit(void) {
  return (((((s32)textRenderer->unk1C[0] >> 3) & 0x1F) << 11) |
          ((((s32)textRenderer->unk1C[1] >> 3) & 0x1F) << 6) |
          ((((s32)textRenderer->unk1C[2] >> 3) & 0x1F) << 1) | 1);
}

void addTextWithCurrentColor(s32 arg0, s32 arg1, char *arg2) {
  s32 temp_v0;

  temp_v0 = loadFont(0);
  arg0 *= textRenderer->unk4[temp_v0].half_width;
  arg1 *= textRenderer->unk4[temp_v0].height;
  addText(arg0, arg1, temp_v0, arg2, textRenderer->unk1C);
}

void addTextWithCurrentColorSimple(s32 arg0, s32 arg1, char *arg2) {
  addText(arg0, arg1, loadFont(0), arg2, textRenderer->unk1C);
}

void addTextWithTemporaryColor(s32 arg0, s32 arg1, char *arg2) {
  u8 sp1C[3];
  void *temp_v0;

  temp_v0 = textRenderer;
  sp1C[0] = textRenderer->unk1C[0];
  sp1C[1] = textRenderer->unk1C[1];
  sp1C[2] = textRenderer->unk1C[2];
  setTextColor(0xC8, 0xEB, 0xD2);
  addTextWithCurrentColor(arg0, arg1, arg2);
  textRenderer->unk1C[0] = sp1C[0];
  textRenderer->unk1C[1] = sp1C[1];
  textRenderer->unk1C[2] = sp1C[2];
}

void calculateTextWidthSimple(char *str) {
  calculateTextWidth(str, loadFont(0), 0, textRenderer->flags);
}

void addCenteredTextWithDefaultColor(s32 arg0, char *arg1) {
  s32 sp2C;
  #ifndef LIGHTHOUSE_P
  u8 sp28[3] = defaultColor5;
  #else
  u8 sp28[3];
  sp28[0] = defaultColor5[0];
  sp28[1] = defaultColor5[1];
  sp28[2] = defaultColor5[2];
  #endif

  sp2C = loadFont(0);
  arg0 *= textRenderer->unk4[sp2C].height;
  addText((s32)(((f32)gFramebufferWidth -
                 (f32)calculateTextWidth(arg1, sp2C, 0, textRenderer->flags)) /
                2),
          arg0, sp2C, arg1, sp28);
}

void addCenteredTextWithCurrentColor(s32 arg0, char *arg1) {
  s32 sp24;
  s32 temp_v0;
  void *temp_v1;

  sp24 = loadFont(0);
  arg0 *= textRenderer->unk4[sp24].height;
  addText((s32)(((f32)gFramebufferWidth -
                 (f32)calculateTextWidth(arg1, sp24, 0, textRenderer->flags)) /
                2),
          arg0, sp24, arg1, textRenderer->unk1C);
}

void enableTextFlag(void) { textRenderer->flags |= 1; }

void disableTextFlag(void) { textRenderer->flags &= ~(0x1); }

void enableTextFlags(s32 arg0) { textRenderer->flags |= arg0 & ~(0x1); }

void disableTextFlags(s32 arg0) { textRenderer->flags &= ~arg0 | 0x1; }

void defragTextRenderer(s32 arg0) {
  BKSprite_s *prev_sprite_ptr;
  s32 chunk_count;
  s32 i_chunk;
  s32 var_s5;
  s32 var_s6;

  if (textRenderer != NULL) {
    for (var_s6 = 0; var_s6 < arg0; var_s6++) {
      textRenderer = (TextRendererSruct *)defrag(textRenderer);
      textRenderer->unk0 = (TextRendererCharacter *)defrag(textRenderer->unk0);
      textRenderer->unk4 = (TextRendererFontInfo *)defrag(textRenderer->unk4);
      textRenderer->string = (char *)defrag(textRenderer->string);
      for (var_s5 = 0; var_s5 < textRenderer->unk10; var_s5++) {
        if (textRenderer->unk4[var_s5].letter_texture != NULL) {
          textRenderer->unk4[var_s5].letter_texture =
              (BKSpriteTextureBlock **)defrag(
                  textRenderer->unk4[var_s5].letter_texture);
        }

        prev_sprite_ptr = textRenderer->unk4[var_s5].font_bin;
        if (textRenderer->unk4[var_s5].font_bin != NULL) {
          chunk_count = sprite_getFramePtr(prev_sprite_ptr, 0U)->chunkCnt;
          textRenderer->unk4[var_s5].font_bin =
              (BKSprite_s *)defrag_asset(textRenderer->unk4[var_s5].font_bin);
          for (i_chunk = 0; i_chunk < chunk_count; i_chunk++) {
            textRenderer->unk4[var_s5].letter_texture[i_chunk] =
                ((u32)(((s32)textRenderer->unk4[var_s5].letter_texture[i_chunk] -
                        (s32)prev_sprite_ptr)) +
                 (u32)textRenderer->unk4[var_s5].font_bin);
          }
        }
      }
    }
  }
}
