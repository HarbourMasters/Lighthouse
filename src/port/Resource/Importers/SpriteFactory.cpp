#include "SpriteFactory.h"
#include "../Type/Sprite.h"
#include "spdlog/spdlog.h"
#include <libultraship/libultraship.h>
#include <fast/resource/type/Texture.h>
#include <algorithm>

namespace Factories {
std::shared_ptr<Ship::IResource>
ResourceFactoryBinarySpriteV0::ReadResource(std::shared_ptr<Ship::File> file,
                                            std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto sprite = std::make_shared<Sprite>(initData);
    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);

    int16_t formatCode = reader->ReadInt16();
    int16_t hdrUnk4 = reader->ReadInt16();
    int16_t hdrUnk6 = reader->ReadInt16();
    int16_t hdrUnk8 = reader->ReadInt16(); // Display width
    int16_t hdrUnkA = reader->ReadInt16(); // Display height
    // Animation parameters from ROM unkC bitfield
    uint8_t animSpeed = reader->ReadUByte();
    uint8_t animType = reader->ReadUByte();
    uint8_t animDirection = reader->ReadUByte();
    uint8_t animFlip = reader->ReadUByte();
    uint32_t positionsSize = reader->ReadUInt32();
    std::vector<std::pair<int16_t, int16_t>> positions;
    for (uint32_t i = 0; i < positionsSize; i++) {
        int16_t x = reader->ReadInt16();
        int16_t y = reader->ReadInt16();
        positions.emplace_back(x, y);
    }
    uint32_t chunkCountsSize = reader->ReadUInt32();
    std::vector<uint16_t> chunkCounts;
    for (uint32_t i = 0; i < chunkCountsSize; i++) {
        uint16_t count = reader->ReadUInt16();
        chunkCounts.push_back(count);
    }

    // Read per-frame header data (added in Torch sprite exporter v2)
    struct FrameHeaderData {
        int16_t x, y, w, h, unkA, unkC, unkE, unk10, unk12;
    };
    std::vector<FrameHeaderData> frameHeaders;
    uint32_t frameHeaderCount = reader->ReadUInt32();
    for (uint32_t i = 0; i < frameHeaderCount; i++) {
        FrameHeaderData fh;
        fh.x = reader->ReadInt16();
        fh.y = reader->ReadInt16();
        fh.w = reader->ReadInt16();
        fh.h = reader->ReadInt16();
        fh.unkA = reader->ReadInt16();
        fh.unkC = reader->ReadInt16();
        fh.unkE = reader->ReadInt16();
        fh.unk10 = reader->ReadInt16();
        fh.unk12 = reader->ReadInt16();
        frameHeaders.push_back(fh);
    }

    // [port] Special sprites: Torch exports ROM sprites with frameCount > 0x100 as
    // single-chunk RGBA16 textures (e.g., map font textures). These have positions but
    // no chunk counts or frame headers. Synthesize 1 frame with 1 chunk so the decomp
    // code can access frames[0] without undefined behavior.
    if (chunkCountsSize == 0 && positionsSize > 0) {
        SPDLOG_INFO("[port] Special sprite detected: fmt=0x{:X} positions={} path='{}'", formatCode, positionsSize,
                    initData->Path);
        chunkCountsSize = 1;
        chunkCounts.push_back(static_cast<uint16_t>(positionsSize));
        formatCode = 0x400; // RGBA16 — Torch always extracts these as RGBA16
    }

    sprite->frameCount = chunkCountsSize;
    sprite->formatType = formatCode;
    sprite->headerUnk4 = hdrUnk4;
    sprite->headerUnk6 = hdrUnk6;
    sprite->headerUnk8 = hdrUnk8;
    sprite->headerUnkA = hdrUnkA;
    sprite->animSpeed = animSpeed;
    sprite->animType = animType;
    sprite->animDirection = animDirection;
    sprite->animFlip = animFlip;

    // Build frames with their chunks
    size_t positionIdx = 0;
    for (uint32_t frameIdx = 0; frameIdx < chunkCountsSize; frameIdx++) {
        SpriteFrameData frameData;
        uint16_t chunkCount = chunkCounts[frameIdx];

        // Initialize frame header from exported data
        if (frameIdx < frameHeaders.size()) {
            const auto& fh = frameHeaders[frameIdx];
            frameData.frameHeader.unk0 = fh.x;
            frameData.frameHeader.unk2 = fh.y;
            frameData.frameHeader.w = fh.w;
            frameData.frameHeader.h = fh.h;
            frameData.frameHeader.unkA = fh.unkA;
            frameData.frameHeader.unkC = fh.unkC;
            frameData.frameHeader.unkE = fh.unkE;
            frameData.frameHeader.unk10 = fh.unk10;
            frameData.frameHeader.unk12 = fh.unk12;
        } else {
            frameData.frameHeader.unk0 = 0;
            frameData.frameHeader.unk2 = 0;
            frameData.frameHeader.w = 0;
            frameData.frameHeader.h = 0;
            frameData.frameHeader.unkA = 0;
            frameData.frameHeader.unkC = 0;
            frameData.frameHeader.unkE = 0;
            frameData.frameHeader.unk10 = 0;
            frameData.frameHeader.unk12 = 0;
        }
        frameData.frameHeader.chunkCnt = chunkCount;

        // Load palette data for color-indexed formats
        if (formatCode == 0x1 || formatCode == 0x4) { // CI4 or CI8
            std::string tlutPath = initData->Path + "_" + std::to_string(frameIdx) + "_TLUT";
            auto tlutTexture = std::static_pointer_cast<Fast::Texture>(
                Ship::Context::GetRawInstance()->GetResourceManager()->LoadResourceProcess(tlutPath));

            if (tlutTexture && tlutTexture->ImageData) {
                frameData.paletteData.resize(tlutTexture->ImageDataSize);
                std::memcpy(frameData.paletteData.data(), tlutTexture->ImageData, tlutTexture->ImageDataSize);
                // SPDLOG_INFO("  Loaded TLUT: {} bytes", tlutTexture->ImageDataSize);
            } else {
                // SPDLOG_WARN("Failed to load TLUT for frame {}: {}", frameIdx, tlutPath);
                // Allocate empty palette as fallback
                size_t paletteSize = (formatCode == 0x1) ? 32 : 512; // CI4: 16 colors, CI8: 256 colors
                frameData.paletteData.resize(paletteSize);
            }
        }

        // Load each chunk for this frame
        for (uint16_t chunkIdx = 0; chunkIdx < chunkCount; chunkIdx++) {
            SpriteFrameData::ChunkData chunkData;

            std::string chunkPath = initData->Path + "_" + std::to_string(frameIdx) + "_" + std::to_string(chunkIdx);
            auto texture = std::static_pointer_cast<Fast::Texture>(
                Ship::Context::GetRawInstance()->GetResourceManager()->LoadResourceProcess(chunkPath));

            if (!texture || !texture->ImageData) {
                // SPDLOG_ERROR("Failed to load texture chunk: {}", chunkPath);
                continue;
            }

            // Set chunk header with position from positions array
            if (positionIdx < positions.size()) {
                chunkData.header.x = positions[positionIdx].first;
                chunkData.header.y = positions[positionIdx].second;
                positionIdx++;
            } else {
                chunkData.header.x = 0;
                chunkData.header.y = 0;
            }
            chunkData.header.w = texture->Width;
            chunkData.header.h = texture->Height;

            // Copy texture data
            chunkData.textureData.resize(texture->ImageDataSize);
            std::memcpy(chunkData.textureData.data(), texture->ImageData, texture->ImageDataSize);

            // RGBA32 rows must be padded to a multiple of 4 pixels
            if (formatCode == 0x800 && texture->Width % 4 != 0 && texture->Width > 0 && texture->Height > 0) {
                const int32_t paddedW = (texture->Width + 3) & ~3;
                std::vector<uint8_t> padded(static_cast<size_t>(paddedW) * texture->Height * 4, 0);
                for (int32_t row = 0; row < texture->Height; row++) {
                    std::memcpy(padded.data() + static_cast<size_t>(row) * paddedW * 4,
                                chunkData.textureData.data() + static_cast<size_t>(row) * texture->Width * 4,
                                static_cast<size_t>(texture->Width) * 4);
                }
                chunkData.textureData = std::move(padded);
                chunkData.header.w = static_cast<int16_t>(paddedW);
            }

            // Split oversized chunks
            const int32_t cw = chunkData.header.w;
            const int32_t ch = chunkData.header.h;
            const size_t texels = static_cast<size_t>(cw) * static_cast<size_t>(ch);
            if (cw > 0 && ch > 0 && texels > 4096) {
                const size_t bpp = chunkData.textureData.size() / texels;
                int32_t stripRows = 4096 / cw;
                if (stripRows < 2) {
                    stripRows = 2;
                }
                const int32_t stride = stripRows - 1;
                const size_t rowBytes = static_cast<size_t>(cw) * bpp;
                int32_t startRow = 0;
                while (startRow < ch) {
                    const int32_t hs = std::min(stripRows, ch - startRow);
                    SpriteFrameData::ChunkData strip;
                    strip.header = chunkData.header;
                    strip.header.y = static_cast<int16_t>(chunkData.header.y + startRow);
                    strip.header.h = static_cast<int16_t>(hs);
                    strip.textureData.assign(chunkData.textureData.begin() + static_cast<size_t>(startRow) * rowBytes,
                                             chunkData.textureData.begin() +
                                                 static_cast<size_t>(startRow + hs) * rowBytes);
                    frameData.chunks.push_back(std::move(strip));
                    if (startRow + hs >= ch) {
                        break;
                    }
                    startRow += stride;
                }
            } else {
                frameData.chunks.push_back(std::move(chunkData));
            }
        }

        // Chunk splitting (and any failed chunk loads) can change the chunk count from the
        // value declared in the manifest; make the frame header reflect what was actually built.
        frameData.frameHeader.chunkCnt = static_cast<uint16_t>(frameData.chunks.size());

        // SPDLOG_INFO("  Frame {} final size: {}x{} with {} chunks loaded",
        //            frameIdx, frameData.frameHeader.w, frameData.frameHeader.h, frameData.chunks.size());
        sprite->frames.push_back(std::move(frameData));
    }

    // SPDLOG_INFO("=== Sprite loading complete: {} frames ===", sprite->frames.size());

    return sprite;
}
} // namespace Factories
