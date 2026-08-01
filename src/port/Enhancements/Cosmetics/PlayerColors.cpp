#include "PlayerColors.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>

#include <libultraship/libultraship.h>

#include "port/UI/cvar_prefixes.h"

extern "C" {
#include "model.h"
#include "enums.h"
#include "core2/modelRender.h"
s32 port_anchor_isConnected(void);
}

namespace {

// ---------------------------------------------------------------------------
// Part tables
//
// Derived from the two player model bins: every CI4 texture and every distinct
// vertex colour was attributed to a bone (via the geo layout) and to a body
// region, then grouped by part. Textures are listed per model because the
// low- and high-poly bins have different texture counts and orderings.
// ---------------------------------------------------------------------------

struct ChannelInfo {
    u8 vanilla[3];
    float minSaturation;
    float minHueDeg, maxHueDeg;
};

const ChannelInfo kChannels[] = {
    /* BANJO_FUR        */ { { 0x83, 0x32, 0x03 }, 0.50f, 5.0f, 50.0f },
    /* BANJO_SHORTS     */ { { 0xFF, 0xF6, 0x20 }, 0.50f, 0.0f, 70.0f },
    /* KAZOOIE_FEATHERS */ { { 0xF6, 0x10, 0x00 }, 0.50f, 0.0f, 70.0f },
    /* KAZOOIE_BEAK     */ { { 0xFE, 0xA1, 0x31 }, 0.12f, 0.0f, 70.0f },
    /* BANJO_BACKPACK   */ { { 0x00, 0x4A, 0xE0 }, 0.35f, 190.0f, 250.0f },
    /* BANJO_SKIN       */ { { 0xEB, 0x83, 0x6D }, 0.15f, 0.0f, 60.0f },
};

struct TextureTable {
    const s16* index;
    s32 count;
};

// ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY
const s16 kLowFur[] = { 12, 13, 15, 18 };
const s16 kLowShorts[] = { 5, 6 };
const s16 kLowFeathers[] = { 14 };
const s16 kLowBeak[] = { 19, 20 };
const s16 kLowSkin[] = { 2, 7 };

// ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY
const s16 kHighFur[] = { 13, 14, 16, 19 };
const s16 kHighShorts[] = { 6, 7 };
const s16 kHighFeathers[] = { 15 };
const s16 kHighBeak[] = { 20, 21 };
const s16 kHighSkin[] = { 3, 8 };

#define TEXTABLE(array) \
    { array, (s32)(sizeof(array) / sizeof((array)[0])) }

// The backpack is shaded entirely by vertex colour. The only blue palettes in
// the model belong to the eyes and Banjo's claws, which must stay put.
const TextureTable kLowTextures[] = { TEXTABLE(kLowFur),  TEXTABLE(kLowShorts), TEXTABLE(kLowFeathers),
                                      TEXTABLE(kLowBeak), { nullptr, 0 },       TEXTABLE(kLowSkin) };
const TextureTable kHighTextures[] = { TEXTABLE(kHighFur),  TEXTABLE(kHighShorts), TEXTABLE(kHighFeathers),
                                       TEXTABLE(kHighBeak), { nullptr, 0 },        TEXTABLE(kHighSkin) };

#undef TEXTABLE

const u32 kFurVerts[] = { 0xFA6F12, 0xCC560B, 0xAE4707, 0x833203, 0x632501, 0x411700, 0x270E00 };
const u32 kShortsVerts[] = { 0xFFFF28, 0xEAEA00, 0xF6B400, 0xCE8700, 0xBA5500, 0x874708 };
const u32 kFeatherVerts[] = { 0xFF3300, 0xD00000, 0x880000, 0x520000, 0xFF5F00, 0xFF8900 };
const u32 kBeakVerts[] = { 0xFFFF9F, 0xFFCF3D, 0xFFC700, 0xFEA131, 0xBA7F4B, 0x923900, 0xC74700, 0x662300 };
const u32 kBackpackVerts[] = { 0x7FA9FF, 0x558DFF, 0x2B71FF, 0x004AE0, 0x003092, 0x002268, 0x001A4E };
const u32 kSkinVerts[] = { 0xFFEAB4, 0xFFBE97, 0xFFA385, 0xEB836D, 0xC66E5C, 0x995447, 0x66372F, 0x45251F };

struct VertexTable {
    const u32* color;
    s32 count;
};

#define VTXTABLE(array) \
    { array, (s32)(sizeof(array) / sizeof((array)[0])) }
const VertexTable kVertexTables[] = { VTXTABLE(kFurVerts),  VTXTABLE(kShortsVerts),   VTXTABLE(kFeatherVerts),
                                      VTXTABLE(kBeakVerts), VTXTABLE(kBackpackVerts), VTXTABLE(kSkinVerts) };
#undef VTXTABLE

// ---------------------------------------------------------------------------
// Color math
// ---------------------------------------------------------------------------

struct Hsv {
    float hue; // degrees, 0..360
    float saturation, value;
};

Hsv ToHsv(float red, float green, float blue) {
    const float maxComponent = std::max({ red, green, blue });
    const float minComponent = std::min({ red, green, blue });
    const float chroma = maxComponent - minComponent;
    Hsv hsv{ 0.0f, maxComponent > 0.0f ? chroma / maxComponent : 0.0f, maxComponent };
    if (chroma > 0.0f) {
        if (maxComponent == red) {
            hsv.hue = 60.0f * std::fmod((green - blue) / chroma, 6.0f);
        } else if (maxComponent == green) {
            hsv.hue = 60.0f * (((blue - red) / chroma) + 2.0f);
        } else {
            hsv.hue = 60.0f * (((red - green) / chroma) + 4.0f);
        }
        if (hsv.hue < 0.0f) {
            hsv.hue += 360.0f;
        }
    }
    return hsv;
}

void FromHsv(const Hsv& hsv, float* red, float* green, float* blue) {
    const float chroma = hsv.value * hsv.saturation;
    const float sector = std::fmod(std::fmod(hsv.hue, 360.0f) + 360.0f, 360.0f) / 60.0f;
    const float ramp = chroma * (1.0f - std::fabs(std::fmod(sector, 2.0f) - 1.0f));
    float highest = 0.0f, middle = 0.0f, lowest = 0.0f;
    switch ((int)sector) {
        case 0:
            highest = chroma;
            middle = ramp;
            break;
        case 1:
            highest = ramp;
            middle = chroma;
            break;
        case 2:
            middle = chroma;
            lowest = ramp;
            break;
        case 3:
            middle = ramp;
            lowest = chroma;
            break;
        case 4:
            highest = ramp;
            lowest = chroma;
            break;
        default:
            highest = chroma;
            lowest = ramp;
            break;
    }
    const float offset = hsv.value - chroma;
    *red = highest + offset;
    *green = middle + offset;
    *blue = lowest + offset;
}

float Clamp01(float value) {
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

// Re-tints one source colour so the channel's vanilla colour lands exactly on `target` and
// every other shade of the part keeps its position in the shading ramp.
void Recolor(const ChannelInfo& channel, const BKColorChannelSetting& target, u8 sourceRed, u8 sourceGreen,
             u8 sourceBlue, u8* outRed, u8* outGreen, u8* outBlue) {
    const Hsv source = ToHsv(sourceRed / 255.0f, sourceGreen / 255.0f, sourceBlue / 255.0f);
    const Hsv vanilla = ToHsv(channel.vanilla[0] / 255.0f, channel.vanilla[1] / 255.0f, channel.vanilla[2] / 255.0f);
    const Hsv chosen = ToHsv(target.r / 255.0f, target.g / 255.0f, target.b / 255.0f);

    float hueOffset = source.hue - vanilla.hue;
    while (hueOffset > 180.0f) {
        hueOffset -= 360.0f;
    }
    while (hueOffset < -180.0f) {
        hueOffset += 360.0f;
    }
    const float rampPosition = std::min(std::fabs(hueOffset), 45.0f) / 45.0f;

    Hsv result;
    result.hue = chosen.hue + (hueOffset < 0.0f ? -1.0f : 1.0f) * rampPosition * 8.0f;
    result.saturation =
        vanilla.saturation > 0.001f ? chosen.saturation * (source.saturation / vanilla.saturation) : chosen.saturation;
    result.value = vanilla.value > 0.001f ? chosen.value * (source.value / vanilla.value) : chosen.value;
    result.saturation = Clamp01(result.saturation * (1.0f - 0.55f * rampPosition));
    result.value = Clamp01(result.value * (1.0f + 0.55f * rampPosition));

    float red, green, blue;
    FromHsv(result, &red, &green, &blue);
    *outRed = (u8)std::lround(Clamp01(red) * 255.0f);
    *outGreen = (u8)std::lround(Clamp01(green) * 255.0f);
    *outBlue = (u8)std::lround(Clamp01(blue) * 255.0f);
}

bool PassesFilter(const ChannelInfo& channel, u8 red, u8 green, u8 blue) {
    const Hsv hsv = ToHsv(red / 255.0f, green / 255.0f, blue / 255.0f);
    if (hsv.value <= 0.0f || hsv.saturation < channel.minSaturation) {
        return false;
    }
    return hsv.hue >= channel.minHueDeg && hsv.hue <= channel.maxHueDeg;
}

// ---------------------------------------------------------------------------
// Per-owner recolored copies
// ---------------------------------------------------------------------------

constexpr size_t kPaletteArenaWords = 128 * 1024; // 1 MiB

void* PaletteArenaTake(size_t bytes) {
    static std::unique_ptr<u64[]> arena;
    static size_t usedWords = 0;

    const size_t wordCount = (bytes + 7) / 8;
    if (wordCount > kPaletteArenaWords) {
        return nullptr;
    }
    if (arena == nullptr) {
        arena = std::make_unique<u64[]>(kPaletteArenaWords);
    }
    if (usedWords + wordCount > kPaletteArenaWords) {
        usedWords = 0;
    }
    u64* region = arena.get() + usedWords;
    usedWords += wordCount;
    return region;
}

struct Variant {
    const void* sourceBin = nullptr;
    u64 colorHash = 0;
    // Palettes live in the ring arena above; vertices are read fresh every draw and are not
    // cached by address, so they can be rebuilt in place.
    void* textures = nullptr;
    std::unique_ptr<u64[]> vertices;
};

std::unordered_map<u64, Variant> sVariants;

u64 VariantKey(u32 ownerKey, s32 modelId) {
    return ((u64)ownerKey << 32) | (u32)modelId;
}

u64 HashColors(const BKPlayerColorSet& colors) {
    u64 hash = 1469598103934665603ull;
    const u8* bytes = (const u8*)&colors;
    for (size_t index = 0; index < sizeof(colors); index++) {
        hash = (hash ^ bytes[index]) * 1099511628211ull;
    }
    return hash;
}

bool AnyEnabled(const BKPlayerColorSet& colors) {
    for (int channelIndex = 0; channelIndex < BK_COLOR_CHANNEL_COUNT; channelIndex++) {
        if (colors.channel[channelIndex].enabled) {
            return true;
        }
    }
    return false;
}

const TextureTable* TexturesForModel(s32 modelId) {
    switch (modelId) {
        case ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY:
            return kLowTextures;
        case ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY:
            return kHighTextures;
        default:
            return nullptr;
    }
}

void RecolorTextures(void* textureBlob, const TextureTable* textureTable, const BKPlayerColorSet& colors) {
    BKTextureList* textureList = (BKTextureList*)textureBlob;
    u8* pixelBase = (u8*)textureList + sizeof(BKTextureList) + (size_t)textureList->count * sizeof(BKTextureInfo);

    for (int channelIndex = 0; channelIndex < BK_COLOR_CHANNEL_COUNT; channelIndex++) {
        if (!colors.channel[channelIndex].enabled) {
            continue;
        }
        const ChannelInfo& channel = kChannels[channelIndex];
        for (s32 entry = 0; entry < textureTable[channelIndex].count; entry++) {
            const s16 textureIndex = textureTable[channelIndex].index[entry];
            if (textureIndex < 0 || textureIndex >= textureList->count) {
                continue;
            }
            BKTextureInfo* texture = &textureList->texture_infos[textureIndex];
            const s32 paletteBytes = textureInfo_getPaletteSize(texture);
            if (paletteBytes <= 0) {
                continue;
            }
            if ((size_t)texture->offset + (size_t)paletteBytes > (size_t)textureList->size) {
                continue;
            }
            // TLUT entries are big-endian RGBA5551, matching what the RDP (and Fast3D's
            // CI4 importer) reads out of DRAM.
            u8* palette = pixelBase + texture->offset;
            for (s32 paletteEntry = 0; paletteEntry < paletteBytes / 2; paletteEntry++) {
                const u16 packedColor = (u16)((palette[paletteEntry * 2] << 8) | palette[paletteEntry * 2 + 1]);
                const u8 red = (u8)(((packedColor >> 11) & 0x1F) * 255 / 31);
                const u8 green = (u8)(((packedColor >> 6) & 0x1F) * 255 / 31);
                const u8 blue = (u8)(((packedColor >> 1) & 0x1F) * 255 / 31);
                if (!PassesFilter(channel, red, green, blue)) {
                    continue;
                }
                u8 newRed, newGreen, newBlue;
                Recolor(channel, colors.channel[channelIndex], red, green, blue, &newRed, &newGreen, &newBlue);
                const u16 recolored = (u16)(((newRed * 31 / 255) << 11) | ((newGreen * 31 / 255) << 6) |
                                            ((newBlue * 31 / 255) << 1) | (packedColor & 1));
                palette[paletteEntry * 2] = (u8)(recolored >> 8);
                palette[paletteEntry * 2 + 1] = (u8)(recolored & 0xFF);
            }
        }
    }
}

void RecolorVertices(void* vertexBlob, const BKPlayerColorSet& colors) {
    BKVertexList* vertexList = (BKVertexList*)vertexBlob;
    Vtx* vertices = vertexList->vertices;
    const s32 vertexCount = vertexList->count;

    for (s32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++) {
        Vtx_t& vertex = vertices[vertexIndex].v;
        const u32 packedColor = ((u32)vertex.cn[0] << 16) | ((u32)vertex.cn[1] << 8) | (u32)vertex.cn[2];
        for (int channelIndex = 0; channelIndex < BK_COLOR_CHANNEL_COUNT; channelIndex++) {
            if (!colors.channel[channelIndex].enabled) {
                continue;
            }
            const VertexTable& vertexColors = kVertexTables[channelIndex];
            bool matched = false;
            for (s32 entry = 0; entry < vertexColors.count; entry++) {
                if (vertexColors.color[entry] == packedColor) {
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                continue;
            }
            u8 newRed, newGreen, newBlue;
            Recolor(kChannels[channelIndex], colors.channel[channelIndex], vertex.cn[0], vertex.cn[1], vertex.cn[2],
                    &newRed, &newGreen, &newBlue);
            vertex.cn[0] = newRed;
            vertex.cn[1] = newGreen;
            vertex.cn[2] = newBlue;
            break;
        }
    }
}

Variant* BuildVariant(u32 ownerKey, s32 modelId, BKModelBin* modelBin, const BKPlayerColorSet& colors) {
    const TextureTable* textureTable = TexturesForModel(modelId);
    if (textureTable == nullptr || modelBin == nullptr || modelBin->texture_list_offset == 0 ||
        modelBin->vtx_list_offset == 0) {
        return nullptr;
    }

    Variant& variant = sVariants[VariantKey(ownerKey, modelId)];
    const u64 colorHash = HashColors(colors);
    if (variant.sourceBin == modelBin && variant.colorHash == colorHash && variant.textures != nullptr) {
        return &variant;
    }

    const BKTextureList* sourceTextures = modelbin_getTextureList(modelBin);
    const BKVertexList* sourceVertices = modelbin_getVtxList(modelBin);
    if (sourceTextures == nullptr || sourceVertices == nullptr) {
        return nullptr;
    }

    const size_t textureBytes =
        sizeof(BKTextureList) + (size_t)sourceTextures->count * sizeof(BKTextureInfo) + (size_t)sourceTextures->size;
    const size_t vertexBytes = sizeof(BKVertexList) + (size_t)sourceVertices->count * sizeof(Vtx);

    void* textures = PaletteArenaTake(textureBytes);
    if (textures == nullptr) {
        return nullptr;
    }
    std::memcpy(textures, sourceTextures, textureBytes);

    variant.vertices = std::make_unique<u64[]>((vertexBytes + 7) / 8);
    std::memcpy(variant.vertices.get(), sourceVertices, vertexBytes);

    variant.sourceBin = modelBin;
    variant.colorHash = colorHash;
    variant.textures = textures;

    RecolorTextures(variant.textures, textureTable, colors);
    RecolorVertices(variant.vertices.get(), colors);
    return &variant;
}

const char* kChannelCVars[] = {
    CVAR_REMOTE_ANCHOR("Colors.BanjoFur"),        CVAR_REMOTE_ANCHOR("Colors.BanjoShorts"),
    CVAR_REMOTE_ANCHOR("Colors.KazooieFeathers"), CVAR_REMOTE_ANCHOR("Colors.KazooieBeak"),
    CVAR_REMOTE_ANCHOR("Colors.BanjoBackpack"),   CVAR_REMOTE_ANCHOR("Colors.BanjoSkin"),
};

#define CHANNEL_TABLE_COMPLETE(table) \
    static_assert(sizeof(table) / sizeof((table)[0]) == BK_COLOR_CHANNEL_COUNT, #table)
CHANNEL_TABLE_COMPLETE(kChannelCVars);
CHANNEL_TABLE_COMPLETE(kChannels);
CHANNEL_TABLE_COMPLETE(kVertexTables);
CHANNEL_TABLE_COMPLETE(kLowTextures);
CHANNEL_TABLE_COMPLETE(kHighTextures);
#undef CHANNEL_TABLE_COMPLETE

} // namespace

extern "C" {

const char* PlayerColors_getChannelCVar(s32 channel) {
    if (channel < 0 || channel >= BK_COLOR_CHANNEL_COUNT) {
        return nullptr;
    }
    return kChannelCVars[channel];
}

void PlayerColors_getVanilla(s32 channel, u8* red, u8* green, u8* blue) {
    if (channel < 0 || channel >= BK_COLOR_CHANNEL_COUNT) {
        return;
    }
    *red = kChannels[channel].vanilla[0];
    *green = kChannels[channel].vanilla[1];
    *blue = kChannels[channel].vanilla[2];
}

void PlayerColors_getLocal(BKPlayerColorSet* out) {
    if (out == nullptr) {
        return;
    }
    for (int channelIndex = 0; channelIndex < BK_COLOR_CHANNEL_COUNT; channelIndex++) {
        const ChannelInfo& channel = kChannels[channelIndex];
        const std::string valueCVar = std::string(kChannelCVars[channelIndex]) + ".Value";
        const std::string enabledCVar = std::string(kChannelCVars[channelIndex]) + ".Enabled";
        const Color_RGBA8 vanilla = { channel.vanilla[0], channel.vanilla[1], channel.vanilla[2], 255 };
        const Color_RGBA8 chosen = CVarGetColor(valueCVar.c_str(), vanilla);
        out->channel[channelIndex].enabled = CVarGetInteger(enabledCVar.c_str(), 0) ? 1 : 0;
        out->channel[channelIndex].r = chosen.r;
        out->channel[channelIndex].g = chosen.g;
        out->channel[channelIndex].b = chosen.b;
    }
}

void PlayerColors_applyForDraw(u32 ownerKey, s32 modelId, void* modelBin, const BKPlayerColorSet* colors) {
    if (!port_anchor_isConnected()) {
        return;
    }
    if (colors == nullptr || modelBin == nullptr || !AnyEnabled(*colors)) {
        return;
    }
    Variant* variant = BuildVariant(ownerKey, modelId, (BKModelBin*)modelBin, *colors);
    if (variant == nullptr) {
        return;
    }
    modelRender_setTextureList((BKTextureList*)variant->textures);
    modelRender_setVertexList((BKVertexList*)variant->vertices.get());
}

void PlayerColors_applyLocalForDraw(s32 modelId, void* modelBin) {
    if (TexturesForModel(modelId) == nullptr) {
        return;
    }
    BKPlayerColorSet colors;
    PlayerColors_getLocal(&colors);
    PlayerColors_applyForDraw(BK_COLORS_LOCAL_OWNER, modelId, modelBin, &colors);
}

void PlayerColors_forgetOwner(u32 ownerKey) {
    for (auto entry = sVariants.begin(); entry != sVariants.end();) {
        entry = ((u32)(entry->first >> 32) == ownerKey) ? sVariants.erase(entry) : std::next(entry);
    }
}

void PlayerColors_reset(void) {
    sVariants.clear();
}

} // extern "C"
