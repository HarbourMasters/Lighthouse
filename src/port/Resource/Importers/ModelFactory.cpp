#include "ModelFactory.h"
#include "spdlog/spdlog.h"
#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>
#include <fast/resource/type/Vertex.h>
#include <fast/resource/type/Texture.h>

#include "port/Resource/Alt/AltPathPool.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "core2/model.h"

#include "BlobWriter.h"

namespace Factories {
namespace {

void PadTo8(std::vector<uint8_t>& dst) {
    while (dst.size() & 7)
        dst.push_back(0);
}

} // anonymous namespace

// The N of a G_SETTIMG image ReadResource points at "__OTR__<model>_tex_N", or -1 for any other image.
int ModelTextureIndex(uintptr_t image) {
    if (image == 0 || (image & 1) != 0 || std::strncmp((const char*)image, "__OTR__", 7) != 0) {
        return -1;
    }
    const char* tag = nullptr;
    for (const char* found = std::strstr((const char*)image, "_tex_"); found != nullptr;
         found = std::strstr(found + 1, "_tex_")) {
        tag = found;
    }
    return tag != nullptr ? std::atoi(tag + 5) : -1;
}

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryModelV0::ReadResource(std::shared_ptr<Ship::File> file,
                                           std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);
    auto resourceMgr = Ship::Context::GetRawInstance()->GetResourceManager();

    // ── Read metadata written by ModelBinaryExporter::Export ────────────────

    const uint16_t geoType = reader->ReadUInt16();
    const uint16_t triCount = reader->ReadUInt16();
    const uint16_t vertCount = reader->ReadUInt16();
    (void)triCount;
    (void)vertCount;

    const bool hasGeo = reader->ReadUByte() != 0;
    const bool hasVtx = reader->ReadUByte() != 0;
    const bool hasDL = reader->ReadUByte() != 0;
    const uint16_t texCount = reader->ReadUInt16();
    const bool hasAnim = reader->ReadUByte() != 0;
    const bool hasCol = reader->ReadUByte() != 0;
    const bool hasCollisionShapes = reader->ReadUByte() != 0;
    const bool hasCameraAreas = reader->ReadUByte() != 0;
    const bool hasMeshList = reader->ReadUByte() != 0;
    const bool hasVertexBoneMap = reader->ReadUByte() != 0;
    const bool hasAnimTex = reader->ReadUByte() != 0;

    // [port] Debug: dump parsed flags so we can verify stream alignment
    SPDLOG_TRACE("[BKModel] '{}' flags: geoType={} tri={} vert={} geo={} vtx={} dl={} tex={} anim={} col={} shapes={} "
                 "cameraAreas={} meshes={} vertexBoneMap={} animTex={}",
                 initData->Path, geoType, triCount, vertCount, hasGeo, hasVtx, hasDL, texCount, hasAnim, hasCol,
                 hasCollisionShapes, hasCameraAreas, hasMeshList, hasVertexBoneMap, hasAnimTex);

    // VTX header fields
    int16_t vtxMin[3] = {}, vtxMax[3] = {}, vtxCenter[3] = {};
    int16_t vtxLocalNorm = 0, vtxGlobalNorm = 0;
    uint16_t vtxCount = 0;
    if (hasVtx) {
        vtxMin[0] = reader->ReadInt16();
        vtxMin[1] = reader->ReadInt16();
        vtxMin[2] = reader->ReadInt16();
        vtxMax[0] = reader->ReadInt16();
        vtxMax[1] = reader->ReadInt16();
        vtxMax[2] = reader->ReadInt16();
        vtxCenter[0] = reader->ReadInt16();
        vtxCenter[1] = reader->ReadInt16();
        vtxCenter[2] = reader->ReadInt16();
        vtxLocalNorm = reader->ReadInt16();
        vtxCount = reader->ReadUInt16();
        vtxGlobalNorm = reader->ReadInt16();
    }

    // GFX info + raw N64 display list command words
    uint32_t dlCount = 0, dlUnkInfo = 0, gfxSubCount = 0;
    std::vector<uint32_t> rawDLWords;
    if (hasDL) {
        dlCount = reader->ReadUInt32();
        dlUnkInfo = reader->ReadUInt32();
        gfxSubCount = reader->ReadUInt32();
        SPDLOG_TRACE("[BKModel] '{}' DL: count={} unkInfo=0x{:X} subCount={} (need {} more bytes for raw words)",
                     initData->Path, dlCount, dlUnkInfo, gfxSubCount, dlCount * 2 * 4);
        // Read raw N64 DL words embedded by Torch (w0,w1 pairs, dlCount commands)
        rawDLWords.resize(dlCount * 2);
        for (uint32_t i = 0; i < dlCount * 2; i++) {
            rawDLWords[i] = reader->ReadUInt32();
        }
    }

    // Texture metadata
    struct TexMeta {
        uint16_t type;
        uint8_t width, height;
        uint16_t tlutColors;
        uint32_t textureDataOffset;
    };
    std::vector<TexMeta> texMetas(texCount);
    for (uint16_t ti = 0; ti < texCount; ti++) {
        auto& tm = texMetas[ti];
        tm.type = reader->ReadUInt16();
        tm.width = reader->ReadUByte();
        tm.height = reader->ReadUByte();
        tm.tlutColors = reader->ReadUInt16();
        tm.textureDataOffset = reader->ReadUInt32();
        SPDLOG_TRACE("[BKModel] '{}' tex[{}]: type=0x{:X} {}x{} tlutColors={} romOff=0x{:X}", initData->Path, ti,
                     tm.type, tm.width, tm.height, tm.tlutColors, tm.textureDataOffset);
    }

    // Let enhancements patch the raw display list before it is widened.
    // Command indices must be preserved; the geo layout references them by position.
    if (!rawDLWords.empty() && texCount > 0) {
        std::vector<ModelTexSize> texSizes;
        texSizes.reserve(texMetas.size());
        for (const auto& tm : texMetas) {
            texSizes.push_back(ModelTexSize{ tm.width, tm.height });
        }
        CALL_EVENT(OnModelDisplayListLoad, initData->Path.c_str(), rawDLWords.data(),
                   static_cast<uint32_t>(rawDLWords.size()), texSizes.data(), texCount);
    }

    // Raw texture data blob — full contiguous pixel area from the ROM.
    // Preserves animated texture frames and unlisted data between textures.
    uint32_t rawTexDataSize = reader->ReadUInt32();
    std::vector<uint8_t> rawTexBlob;
    if (rawTexDataSize > 0) {
        rawTexBlob.resize(rawTexDataSize);
        for (uint32_t i = 0; i < rawTexDataSize; i++)
            rawTexBlob[i] = reader->ReadInt8();
    }

    // Animation
    float animScale = 0.0f;
    uint16_t boneCount = 0;
    struct BoneEntry {
        float pos[3];
        uint16_t id, parentId;
    };
    std::vector<BoneEntry> bones;
    if (hasAnim) {
        animScale = reader->ReadFloat();
        boneCount = reader->ReadUInt16();
        for (uint16_t i = 0; i < boneCount; i++) {
            BoneEntry b{};
            b.pos[0] = reader->ReadFloat();
            b.pos[1] = reader->ReadFloat();
            b.pos[2] = reader->ReadFloat();
            b.id = reader->ReadUInt16();
            b.parentId = reader->ReadUInt16();
            bones.push_back(b);
        }
    }

    // Collision
    int16_t colMin[3] = {}, colMax[3] = {};
    uint16_t colYStride = 0, colZStride = 0, colCubeScale = 0;
    struct ColCubeEntry {
        uint16_t startTri, triCount;
    };
    struct ColTriEntry {
        uint16_t vtxIds[3], unk6;
        uint32_t flags;
    };
    std::vector<ColCubeEntry> colCubes;
    std::vector<ColTriEntry> colTris;
    if (hasCol) {
        SPDLOG_TRACE("[BKModel] '{}' reading collision...", initData->Path);
        colMin[0] = reader->ReadInt16();
        colMin[1] = reader->ReadInt16();
        colMin[2] = reader->ReadInt16();
        colMax[0] = reader->ReadInt16();
        colMax[1] = reader->ReadInt16();
        colMax[2] = reader->ReadInt16();
        colYStride = reader->ReadUInt16();
        colZStride = reader->ReadUInt16();
        colCubeScale = reader->ReadUInt16();
        const uint16_t cubeCnt = reader->ReadUInt16();
        const uint16_t triCnt = reader->ReadUInt16();
        SPDLOG_TRACE("[BKModel] '{}' collision: cubes={} tris={}", initData->Path, cubeCnt, triCnt);
        for (uint16_t i = 0; i < cubeCnt; i++) {
            ColCubeEntry e{};
            e.startTri = reader->ReadUInt16();
            e.triCount = reader->ReadUInt16();
            colCubes.push_back(e);
        }
        for (uint16_t i = 0; i < triCnt; i++) {
            ColTriEntry e{};
            e.vtxIds[0] = reader->ReadUInt16();
            e.vtxIds[1] = reader->ReadUInt16();
            e.vtxIds[2] = reader->ReadUInt16();
            e.unk6 = reader->ReadUInt16();
            e.flags = reader->ReadUInt32();
            colTris.push_back(e);
        }
    }

    int16_t shapeBoxCount = 0, shapeCylinderCount = 0, shapeSphereCount = 0, shapeCullRadius = 0;
    struct ShapeBox {
        int16_t scale1[3], scale2[3], pos[3];
        uint8_t rot[3], unk15;
        int8_t animIdx;
        uint8_t pad;
    };
    struct ShapeCylinder {
        int16_t unk0, unk2, pos[3];
        uint8_t rot[3], unkD;
        int8_t animIdx;
        uint8_t pad;
    };
    struct ShapeSphere {
        int16_t unk0, unk2[3];
        uint8_t unk8;
        int8_t unk9;
        uint8_t pad[2];
    };
    std::vector<ShapeBox> shapeBoxes;
    std::vector<ShapeCylinder> shapeCylinders;
    std::vector<ShapeSphere> shapeSpheres;
    if (hasCollisionShapes) {
        shapeBoxCount = reader->ReadInt16();
        shapeCylinderCount = reader->ReadInt16();
        shapeSphereCount = reader->ReadInt16();
        shapeCullRadius = reader->ReadInt16();
        for (int16_t i = 0; i < shapeBoxCount; i++) {
            ShapeBox e{};
            e.scale1[0] = reader->ReadInt16();
            e.scale1[1] = reader->ReadInt16();
            e.scale1[2] = reader->ReadInt16();
            e.scale2[0] = reader->ReadInt16();
            e.scale2[1] = reader->ReadInt16();
            e.scale2[2] = reader->ReadInt16();
            e.pos[0] = reader->ReadInt16();
            e.pos[1] = reader->ReadInt16();
            e.pos[2] = reader->ReadInt16();
            e.rot[0] = reader->ReadUByte();
            e.rot[1] = reader->ReadUByte();
            e.rot[2] = reader->ReadUByte();
            e.unk15 = reader->ReadUByte();
            e.animIdx = reader->ReadInt8();
            e.pad = reader->ReadUByte();
            shapeBoxes.push_back(e);
        }
        for (int16_t i = 0; i < shapeCylinderCount; i++) {
            ShapeCylinder e{};
            e.unk0 = reader->ReadInt16();
            e.unk2 = reader->ReadInt16();
            e.pos[0] = reader->ReadInt16();
            e.pos[1] = reader->ReadInt16();
            e.pos[2] = reader->ReadInt16();
            e.rot[0] = reader->ReadUByte();
            e.rot[1] = reader->ReadUByte();
            e.rot[2] = reader->ReadUByte();
            e.unkD = reader->ReadUByte();
            e.animIdx = reader->ReadInt8();
            e.pad = reader->ReadUByte();
            shapeCylinders.push_back(e);
        }
        for (int16_t i = 0; i < shapeSphereCount; i++) {
            ShapeSphere e{};
            e.unk0 = reader->ReadInt16();
            e.unk2[0] = reader->ReadInt16();
            e.unk2[1] = reader->ReadInt16();
            e.unk2[2] = reader->ReadInt16();
            e.unk8 = reader->ReadUByte();
            e.unk9 = reader->ReadInt8();
            e.pad[0] = reader->ReadUByte();
            e.pad[1] = reader->ReadUByte();
            shapeSpheres.push_back(e);
        }
    }

    uint8_t cameraAreaCount = 0;
    struct CameraArea {
        int16_t unk0[3], unk6[3];
        uint8_t unkC, pad;
    };
    std::vector<CameraArea> cameraAreas;
    if (hasCameraAreas) {
        cameraAreaCount = reader->ReadUByte();
        for (uint8_t i = 0; i < cameraAreaCount; i++) {
            CameraArea e{};
            e.unk0[0] = reader->ReadInt16();
            e.unk0[1] = reader->ReadInt16();
            e.unk0[2] = reader->ReadInt16();
            e.unk6[0] = reader->ReadInt16();
            e.unk6[1] = reader->ReadInt16();
            e.unk6[2] = reader->ReadInt16();
            e.unkC = reader->ReadUByte();
            e.pad = reader->ReadUByte();
            cameraAreas.push_back(e);
        }
    }

    uint16_t meshCount = 0;
    struct MeshEntry {
        uint16_t dataInfo;
        std::vector<uint16_t> vtxIndices;
    };
    std::vector<MeshEntry> meshes;
    if (hasMeshList) {
        try {
            meshCount = reader->ReadUInt16();
            for (uint16_t i = 0; i < meshCount; i++) {
                MeshEntry e{};
                e.dataInfo = reader->ReadUInt16();
                const uint16_t vtxCnt = reader->ReadUInt16();
                for (uint16_t j = 0; j < vtxCnt; j++)
                    e.vtxIndices.push_back(reader->ReadUInt16());
                meshes.push_back(e);
            }
        } catch (const std::out_of_range& ex) {
            SPDLOG_WARN("[BKModel] Truncated mesh list in {}, ignoring it: {}", initData->Path, ex.what());
            meshes.clear();
        }
    }

    int16_t vertexBoneCount = 0;
    struct VertexBoneEntry {
        int16_t coord[3];
        int8_t animIdx;
        std::vector<int16_t> vtxList;
    };
    std::vector<VertexBoneEntry> vertexBoneMap;
    if (hasVertexBoneMap) {
        vertexBoneCount = reader->ReadInt16();
        for (int16_t i = 0; i < vertexBoneCount; i++) {
            VertexBoneEntry e{};
            e.coord[0] = reader->ReadInt16();
            e.coord[1] = reader->ReadInt16();
            e.coord[2] = reader->ReadInt16();
            e.animIdx = reader->ReadInt8();
            const uint8_t vtxCnt = reader->ReadUByte();
            for (uint8_t j = 0; j < vtxCnt; j++)
                e.vtxList.push_back(reader->ReadInt16());
            vertexBoneMap.push_back(e);
        }
    }

    // AnimTextures (always 4 slots)
    struct AnimTexEntry {
        uint16_t frameSize, frameCount;
        float frameRate;
    };
    std::vector<AnimTexEntry> animTextures;
    if (hasAnimTex) {
        for (int i = 0; i < 4; i++) {
            AnimTexEntry at{};
            at.frameSize = reader->ReadUInt16();
            at.frameCount = reader->ReadUInt16();
            at.frameRate = reader->ReadFloat();
            animTextures.push_back(at);
        }
    }

    // ── Load GEO sub-resource (GeoLayout render tree) ─────────────────────────
    std::shared_ptr<Ship::Blob> geoBlob;
    if (hasGeo) {
        SPDLOG_TRACE("[BKModel] '{}' loading _GEO sub-resource...", initData->Path);
        geoBlob = std::static_pointer_cast<Ship::Blob>(resourceMgr->LoadResourceProcess(initData->Path + "_GEO"));
        if (!geoBlob) {
            SPDLOG_WARN("[BKModel] _GEO not found: {}", initData->Path);
        }
    }

    // ── Assemble contiguous BKModelBin buffer ─────────────────────────────────
    //
    // Layout (each section padded to 8-byte alignment):
    //   [  0 ] BKModelBin header (zero-init, geo_type set)
    //   [ T  ] BKTextureList + BKTextureInfo[] + pixel data    -> texture_list_offset
    //   [ GL ] GeoLayout command tree                          -> geo_list_offset
    //   [ A  ] BKAnimationList + BKAnimation[]                 -> animation_list_offset
    //   [ B  ] BKCollisionList + ColGeo[] + ColTri[]           -> collision_list_offset
    //   [ CV ] collision volumes, boxes/cylinders/spheres      -> unk14_list_offset
    //   [ CA ] BKCameraAreaList + entries                      -> camera_area_list_offset
    //   [ ML ] mesh count (s16) + named vertex groups          -> mesh_list_offset
    //   [ VB ] vertex to bone binding + entries                -> anim_vertices_list_offset
    //   [ AT ] AnimTexture[4]                                  -> animated_texture_list_offset
    //   [ V  ] BKVertexList header + Vtx[]                     -> vtx_list_offset
    //   [ G  ] BKGfxList header + Gfx[]                        -> gfx_list_offset

    auto out = std::vector<uint8_t>(sizeof(BKModelBin), 0);
    auto hdr = [&]() -> BKModelBin* { return reinterpret_cast<BKModelBin*>(out.data()); };
    hdr()->geo_type = static_cast<int16_t>(geoType);

    // Texture section
    if (texCount > 0) {
        PadTo8(out);
        hdr()->texture_list_offset = static_cast<int16_t>(out.size());

        // Use raw texture blob directly — preserves animated frames and
        // all unlisted data. OTEX overlay for alt assets can be added later.
        const uint8_t* blobPtr = rawTexBlob.empty() ? nullptr : rawTexBlob.data();
        uint32_t totalPixSize = rawTexDataSize;

        // BKTextureList: s32 size_0, s16 cnt_4, u8 pad[2]
        AppendValue<int32_t>(out, static_cast<int32_t>(totalPixSize));
        AppendValue<int16_t>(out, static_cast<int16_t>(texCount));
        AppendValue<uint8_t>(out, 0);
        AppendValue<uint8_t>(out, 0);

        // BKTextureInfo[] (16 bytes each) — use original ROM offsets
        for (uint16_t i = 0; i < texCount; i++) {
            const auto& tm = texMetas[i];
            AppendValue<int32_t>(out, static_cast<int32_t>(tm.textureDataOffset));
            AppendValue<int16_t>(out, static_cast<int16_t>(tm.type));
            AppendValue<uint8_t>(out, 0);
            AppendValue<uint8_t>(out, 0);
            AppendValue<uint8_t>(out, tm.width);
            AppendValue<uint8_t>(out, tm.height);
            for (int p = 0; p < 6; p++)
                AppendValue<uint8_t>(out, 0);
        }

        // Write pixel data area (raw blob from Torch)
        if (blobPtr) {
            AppendBytes(out, blobPtr, rawTexDataSize);
        }
    }

    // GeoLayout section
    if (geoBlob && !geoBlob->Data.empty()) {
        PadTo8(out);
        hdr()->geo_list_offset = static_cast<int32_t>(out.size());
        AppendBytes(out, geoBlob->Data.data(), geoBlob->Data.size());
    }

    // Animation section
    if (hasAnim) {
        PadTo8(out);
        hdr()->animation_list_offset = static_cast<int32_t>(out.size());

        // BKAnimationList: f32 scalingFactor, s16 boneCount, u8 pad[2], BKAnimation[]
        AppendValue<float>(out, animScale);
        AppendValue<int16_t>(out, static_cast<int16_t>(boneCount));
        AppendValue<uint8_t>(out, 0);
        AppendValue<uint8_t>(out, 0);
        for (const auto& b : bones) {
            AppendValue<float>(out, b.pos[0]);
            AppendValue<float>(out, b.pos[1]);
            AppendValue<float>(out, b.pos[2]);
            AppendValue<int16_t>(out, static_cast<int16_t>(b.id));
            AppendValue<int16_t>(out, static_cast<int16_t>(b.parentId));
        }
    }

    // Collision section
    if (hasCol) {
        PadTo8(out);
        hdr()->collision_list_offset = static_cast<int32_t>(out.size());

        // BKCollisionList header (24 bytes)
        AppendValue<int16_t>(out, colMin[0]);
        AppendValue<int16_t>(out, colMin[1]);
        AppendValue<int16_t>(out, colMin[2]);
        AppendValue<int16_t>(out, colMax[0]);
        AppendValue<int16_t>(out, colMax[1]);
        AppendValue<int16_t>(out, colMax[2]);
        AppendValue<int16_t>(out, static_cast<int16_t>(colYStride));
        AppendValue<int16_t>(out, static_cast<int16_t>(colZStride));
        AppendValue<int16_t>(out, static_cast<int16_t>(colCubes.size()));
        AppendValue<int16_t>(out, static_cast<int16_t>(colCubeScale));
        AppendValue<int16_t>(out, static_cast<int16_t>(colTris.size()));
        AppendValue<uint8_t>(out, 0);
        AppendValue<uint8_t>(out, 0);

        for (const auto& gc : colCubes) {
            AppendValue<int16_t>(out, static_cast<int16_t>(gc.startTri));
            AppendValue<int16_t>(out, static_cast<int16_t>(gc.triCount));
        }
        for (const auto& ct : colTris) {
            AppendValue<int16_t>(out, static_cast<int16_t>(ct.vtxIds[0]));
            AppendValue<int16_t>(out, static_cast<int16_t>(ct.vtxIds[1]));
            AppendValue<int16_t>(out, static_cast<int16_t>(ct.vtxIds[2]));
            AppendValue<int16_t>(out, static_cast<int16_t>(ct.unk6));
            AppendValue<int32_t>(out, static_cast<int32_t>(ct.flags));
        }
    }

    if (hasCollisionShapes) {
        PadTo8(out);
        hdr()->unk14_list_offset = static_cast<int32_t>(out.size());

        AppendValue<int16_t>(out, shapeBoxCount);
        AppendValue<int16_t>(out, shapeCylinderCount);
        AppendValue<int16_t>(out, shapeSphereCount);
        AppendValue<int16_t>(out, shapeCullRadius);
        for (const auto& e : shapeBoxes) {
            AppendValue<int16_t>(out, e.scale1[0]);
            AppendValue<int16_t>(out, e.scale1[1]);
            AppendValue<int16_t>(out, e.scale1[2]);
            AppendValue<int16_t>(out, e.scale2[0]);
            AppendValue<int16_t>(out, e.scale2[1]);
            AppendValue<int16_t>(out, e.scale2[2]);
            AppendValue<int16_t>(out, e.pos[0]);
            AppendValue<int16_t>(out, e.pos[1]);
            AppendValue<int16_t>(out, e.pos[2]);
            AppendValue<uint8_t>(out, e.rot[0]);
            AppendValue<uint8_t>(out, e.rot[1]);
            AppendValue<uint8_t>(out, e.rot[2]);
            AppendValue<uint8_t>(out, e.unk15);
            AppendValue<int8_t>(out, e.animIdx);
            AppendValue<uint8_t>(out, e.pad);
        }
        for (const auto& e : shapeCylinders) {
            AppendValue<int16_t>(out, e.unk0);
            AppendValue<int16_t>(out, e.unk2);
            AppendValue<int16_t>(out, e.pos[0]);
            AppendValue<int16_t>(out, e.pos[1]);
            AppendValue<int16_t>(out, e.pos[2]);
            AppendValue<uint8_t>(out, e.rot[0]);
            AppendValue<uint8_t>(out, e.rot[1]);
            AppendValue<uint8_t>(out, e.rot[2]);
            AppendValue<uint8_t>(out, e.unkD);
            AppendValue<int8_t>(out, e.animIdx);
            AppendValue<uint8_t>(out, e.pad);
        }
        for (const auto& e : shapeSpheres) {
            AppendValue<int16_t>(out, e.unk0);
            AppendValue<int16_t>(out, e.unk2[0]);
            AppendValue<int16_t>(out, e.unk2[1]);
            AppendValue<int16_t>(out, e.unk2[2]);
            AppendValue<uint8_t>(out, e.unk8);
            AppendValue<int8_t>(out, e.unk9);
            AppendValue<uint8_t>(out, e.pad[0]);
            AppendValue<uint8_t>(out, e.pad[1]);
        }
    }

    if (hasCameraAreas) {
        PadTo8(out);
        hdr()->camera_area_list_offset = static_cast<int32_t>(out.size());

        AppendValue<uint8_t>(out, cameraAreaCount);
        AppendValue<uint8_t>(out, 0);
        for (const auto& e : cameraAreas) {
            AppendValue<int16_t>(out, e.unk0[0]);
            AppendValue<int16_t>(out, e.unk0[1]);
            AppendValue<int16_t>(out, e.unk0[2]);
            AppendValue<int16_t>(out, e.unk6[0]);
            AppendValue<int16_t>(out, e.unk6[1]);
            AppendValue<int16_t>(out, e.unk6[2]);
            AppendValue<uint8_t>(out, e.unkC);
            AppendValue<uint8_t>(out, e.pad);
        }
    }

    if (hasMeshList && !meshes.empty()) {
        PadTo8(out);
        hdr()->mesh_list_offset = static_cast<int32_t>(out.size());

        AppendValue<int16_t>(out, static_cast<int16_t>(meshes.size()));
        for (const auto& entry : meshes) {
            AppendValue<uint16_t>(out, entry.dataInfo);
            AppendValue<uint16_t>(out, static_cast<uint16_t>(entry.vtxIndices.size()));
            for (auto idx : entry.vtxIndices)
                AppendValue<uint16_t>(out, idx);
        }
    }

    if (hasVertexBoneMap) {
        PadTo8(out);
        hdr()->anim_vertices_list_offset = static_cast<int32_t>(out.size());

        AppendValue<int16_t>(out, vertexBoneCount);
        AppendValue<uint8_t>(out, 0);
        AppendValue<uint8_t>(out, 0);
        for (const auto& e : vertexBoneMap) {
            AppendValue<int16_t>(out, e.coord[0]);
            AppendValue<int16_t>(out, e.coord[1]);
            AppendValue<int16_t>(out, e.coord[2]);
            AppendValue<int8_t>(out, e.animIdx);
            AppendValue<int8_t>(out, static_cast<int8_t>(e.vtxList.size()));
            for (auto idx : e.vtxList)
                AppendValue<int16_t>(out, idx);
        }
    }

    // AnimTexture section
    if (hasAnimTex) {
        PadTo8(out);
        hdr()->animated_texture_list_offset = static_cast<int32_t>(out.size());

        for (const auto& at : animTextures) {
            AppendValue<int16_t>(out, static_cast<int16_t>(at.frameSize));
            AppendValue<int16_t>(out, static_cast<int16_t>(at.frameCount));
            AppendValue<float>(out, at.frameRate);
        }
    }

    // GFX section — widen raw N64 commands (8 bytes each) to native Gfx (16 bytes each).
    // This preserves the exact N64 command count and indices so geo layout references
    // (list[cmd->unk8]) remain correct. No OTR markers or expanded commands.
    if (hasDL && !rawDLWords.empty()) {
        PadTo8(out);
        hdr()->gfx_list_offset = static_cast<int32_t>(out.size());

        // BKGfxList: s32 dlCount, s32 dlUnkInfo
        AppendValue<int32_t>(out, static_cast<int32_t>(dlCount));
        AppendValue<int32_t>(out, static_cast<int32_t>(dlUnkInfo));

        // Widen each N64 command: (u32 w0, u32 w1) → (uintptr_t w0, uintptr_t w1)
        for (uint32_t i = 0; i < dlCount; i++) {
            uintptr_t w0 = rawDLWords[i * 2];
            uintptr_t w1 = rawDLWords[i * 2 + 1];

            uint8_t opcode = (uint8_t)(w0 >> 24);

            if (opcode == 0xFD && (uint32_t)((w1 >> 24) & 0xFF) == 0xFF) {
                const uint32_t texIndex = (uint32_t)(w1 & 0x00FFFFFF);
                w1 = (uintptr_t)InternAltPath("__OTR__" + initData->Path + "_tex_" + std::to_string(texIndex));
                AppendValue<uintptr_t>(out, w0);
                AppendValue<uintptr_t>(out, w1);
                continue;
            }

            // F3DEX opcodes that carry a segmented address in w1.
            // G_DL byte offsets must be scaled for widened Gfx (16 bytes vs N64's 8).
            if (opcode == 0x06 && sizeof(uintptr_t) > 4) { // G_DL
                if (w1 != 0 && (w1 >> 24) != 0 && w1 <= 0xFFFFFFFF) {
                    uint32_t seg = (uint32_t)(w1 >> 24);
                    uint32_t off = (uint32_t)(w1 & 0x00FFFFFF);
                    off *= (sizeof(Gfx) / 8);
                    w1 = ((uintptr_t)seg << 24) | (off & 0x00FFFFFF);
                }
            }
            // G_VTX (F3DEX opcode 0x04): w1 is a segmented address into the
            // vertex buffer. With GBI_FLOATS, sizeof(Vtx) = 24 instead of N64's 16,
            // so byte offsets must be scaled: new_off = (old_off / 16) * sizeof(Vtx).
            if (opcode == 0x04 && sizeof(Vtx) != 16) {
                if (w1 != 0 && (w1 >> 24) != 0 && w1 <= 0xFFFFFFFF) {
                    uint32_t seg = (uint32_t)(w1 >> 24);
                    uint32_t off = (uint32_t)(w1 & 0x00FFFFFF);
                    off = (off / 16) * sizeof(Vtx);
                    w1 = ((uintptr_t)seg << 24) | (off & 0x00FFFFFF);
                }
            }

            // Tag segmented addresses with bit 0 so SegAddr() identifies them.
            // Only tag opcodes that carry a segmented address in w1. Other commands
            // (G_SETTILE, G_LOADBLOCK, G_SETCOMBINE, G_SETPRIMCOLOR, etc.) have
            // literal values in w1 that must NOT be modified.
            switch (opcode) {
                case 0x01: // G_MTX
                case 0x03: // G_MOVEMEM
                case 0x04: // G_VTX
                case 0x06: // G_DL
                case 0xFD: // G_SETTIMG (gDPSetTextureImage)
                case 0xFE: // G_SETZIMG
                case 0xFF: // G_SETCIMG
                    if (w1 != 0 && w1 <= 0xFFFFFFFF && (w1 >> 24) != 0) {
                        w1 |= 1;
                    }
                    break;
                default:
                    break;
            }

            AppendValue<uintptr_t>(out, w0);
            AppendValue<uintptr_t>(out, w1);
        }

        // Append gSPEndDisplayList sentinel. Some BK models don't include
        // the terminator in dlCount (the N64 game knew chunk sizes from the geo
        // layout and didn't need it). The port's interpreter walks DLs sequentially
        // until gSPEndDisplayList, so we must provide one.
        uintptr_t endW0 = (uintptr_t)0xB8 << 24; // F3DEX G_ENDDL
        uintptr_t endW1 = 0;
        AppendValue<uintptr_t>(out, endW0);
        AppendValue<uintptr_t>(out, endW1);
    }

    // Vertex section
    if (hasVtx) {
        PadTo8(out);
        hdr()->vtx_list_offset = static_cast<int32_t>(out.size());

        auto vtxRes = std::static_pointer_cast<Fast::Vertex>(resourceMgr->LoadResourceProcess(initData->Path + "_VTX"));

        // Fast::Vertex resource already holds properly-sized Vtx structs
        // (with GBI_FLOATS, sizeof(Vtx) = 24, not the N64's 16 bytes).
        if (vtxRes) {
            const size_t resBytes = vtxRes->GetPointerSize();
            const uint16_t resCount = static_cast<uint16_t>(resBytes / sizeof(Vtx));
            if (resCount != vtxCount) {
                SPDLOG_TRACE("[BKModel] {} VTX count mismatch: model o2r says {} but _VTX resource has {} vertices "
                             "({} bytes, sizeof(Vtx)={}) — using resource-derived count.",
                             initData->Path, vtxCount, resCount, resBytes, sizeof(Vtx));
                vtxCount = resCount;
            }
        }

        // BKVertexList header — must match sizeof(BKVertexList) so vtx_18[] starts
        // at the correct offset. sizeof(BKVertexList) includes trailing padding for
        // Vtx alignment (8 bytes due to long long in Vtx union).
        AppendValue<int16_t>(out, vtxMin[0]);
        AppendValue<int16_t>(out, vtxMin[1]);
        AppendValue<int16_t>(out, vtxMin[2]);
        AppendValue<int16_t>(out, vtxMax[0]);
        AppendValue<int16_t>(out, vtxMax[1]);
        AppendValue<int16_t>(out, vtxMax[2]);
        AppendValue<int16_t>(out, vtxCenter[0]);
        AppendValue<int16_t>(out, vtxCenter[1]);
        AppendValue<int16_t>(out, vtxCenter[2]);
        AppendValue<int16_t>(out, vtxLocalNorm);
        AppendValue<uint16_t>(out, vtxCount);
        AppendValue<int16_t>(out, vtxGlobalNorm);

        // Pad header to sizeof(BKVertexList) so vtx_18[] aligns correctly.
        // BKVertexList has 24 bytes of fields but sizeof may be larger due to
        // flexible array member alignment (Vtx requires 8-byte alignment).
        const size_t headerFieldBytes = 24; // 12 s16 fields = 24 bytes
        const size_t headerSize = sizeof(BKVertexList);
        if (headerSize > headerFieldBytes) {
            for (size_t p = 0; p < headerSize - headerFieldBytes; p++)
                out.push_back(0);
        }

        if (vtxRes) {
            // Fast::Vertex stores Vtx structs (with GBI_FLOATS, ob[] are
            // floats, sizeof(Vtx) = 24 not N64's 16). Copy at native stride so
            // the interpreter can index as F3DVtx* (same layout).
            const Vtx* vtxArray = vtxRes->GetPointer();
            const size_t vtxBytes = static_cast<size_t>(vtxCount) * sizeof(Vtx);
            AppendBytes(out, vtxArray, vtxBytes);
        } else {
            SPDLOG_WARN("[BKModel] _VTX not found: {}", initData->Path);
        }
    }

    auto blob = MakeBlob(initData, std::move(out));
    CALL_EVENT(OnModelBinBuilt, initData->Path.c_str(), blob->Data.data());
    return blob;
}
} // namespace Factories