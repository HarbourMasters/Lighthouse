#include "Sprite.h"
#include <cstring>

extern "C" {
void port_spriteAltRegisterChunk(const void* chunkAddr, const char* path);
void port_spriteAltUnregisterChunk(const void* chunkAddr);
}

namespace Factories {

Sprite::~Sprite() {
    for (const void* addr : mRegisteredChunks) {
        port_spriteAltUnregisterChunk(addr);
    }
}

void Sprite::BuildSpriteStructure() {
    // SPDLOG_INFO("=== Building Sprite Structure ===");
    // SPDLOG_INFO("Frame count: {}, Format type: {}", frameCount, formatType);

    // Calculate total size needed for contiguous buffer
    size_t totalSize = sizeof(BKSprite) + frameCount * sizeof(BKSpriteFrame*);
    std::vector<size_t> frameSizes;
    std::vector<size_t> frameOffsets;

    for (int frameIdx = 0; frameIdx < frameCount; frameIdx++) {
        const auto& frameData = frames[frameIdx];

        size_t frameSize = sizeof(BKSpriteFrame);

        if (!frameData.paletteData.empty()) {
            // Palette must be at Align8(sizeof(BKSpriteFrame)) = offset 24.
            // print_getLettersFromFont aligns (frame_ptr+1) to 8 before reading palette,
            // and spriteRender_drawWithSegment does the same.
            frameSize = Align8(frameSize);
            frameSize += frameData.paletteData.size();
        }

        for (const auto& chunk : frameData.chunks) {
            frameSize += sizeof(BKSpriteTextureBlock);
            frameSize = Align8(frameSize);
            frameSize += chunk.textureData.size();
        }

        frameSizes.push_back(frameSize);
        frameOffsets.push_back(totalSize);
        totalSize += frameSize;
    }

    // Allocate single contiguous buffer for entire sprite
    mSpriteHeader = std::make_unique<uint8_t[]>(totalSize);
    uint8_t* basePtr = mSpriteHeader.get();

    // Setup BKSprite header
    BKSprite* sprite = reinterpret_cast<BKSprite*>(basePtr);
    sprite->frameCnt = frameCount;
    sprite->type = formatType;
    sprite->unk4 = headerUnk4;
    sprite->unk6 = headerUnk6;
    sprite->unk8 = headerUnk8;
    sprite->unkA = headerUnkA;
    sprite->unkC.bit31 = animSpeed;
    sprite->unkC.bit27 = animType;
    sprite->unkC.bit24 = animDirection;
    sprite->unkC.bit22 = animFlip;
    sprite->unkC.pad_bit20 = 0;

    // Build each frame in the contiguous buffer
    for (int frameIdx = 0; frameIdx < frameCount; frameIdx++) {
        const auto& frameData = frames[frameIdx];
        size_t frameOffset = frameOffsets[frameIdx];
        uint8_t* framePtr = basePtr + frameOffset;
        size_t offset = 0;

        // SPDLOG_INFO("--- Frame {} at offset {} ---", frameIdx, frameOffset);
        // SPDLOG_INFO("  Chunks: {}, Frame size: {}x{}", frameData.chunks.size(),
        //             frameData.frameHeader.w, frameData.frameHeader.h);

        // Set frame pointer in BKSprite
        sprite->frames[frameIdx] = reinterpret_cast<BKSpriteFrame*>(framePtr);

        // Write frame header
        BKSpriteFrame* frame = reinterpret_cast<BKSpriteFrame*>(framePtr);
        *frame = frameData.frameHeader;
        offset += sizeof(BKSpriteFrame);

        // Write palette at Align8(sizeof(BKSpriteFrame)) = offset 24.
        // Both print_getLettersFromFont and spriteRender_drawWithSegment walk (frame_ptr+1)
        // up to the next 8-byte alignment before reading the palette.
        if (!frameData.paletteData.empty()) {
            offset = Align8(offset);
            std::memcpy(framePtr + offset, frameData.paletteData.data(), frameData.paletteData.size());
            offset += frameData.paletteData.size();
        }

        // Write chunks
        for (size_t chunkIdx = 0; chunkIdx < frameData.chunks.size(); chunkIdx++) {
            const auto& chunkData = frameData.chunks[chunkIdx];

            size_t chunkHeaderOffset = offset;

            // Write chunk header
            BKSpriteTextureBlock* chunk = reinterpret_cast<BKSpriteTextureBlock*>(framePtr + offset);
            *chunk = chunkData.header;
            offset += sizeof(BKSpriteTextureBlock);

            // Register the HD resource path (if any) for this chunk
            if (!chunkData.resPath.empty()) {
                port_spriteAltRegisterChunk(chunk, chunkData.resPath.c_str());
                mRegisteredChunks.push_back(chunk);
            }

            // Write texture data
            offset = Align8(offset);
            std::memcpy(framePtr + offset, chunkData.textureData.data(), chunkData.textureData.size());

            // [port] RGBA16 texture data stays in N64 big-endian byte order.
            // The fast3d interpreter reads RGBA5551 as BE when processing
            // gDPLoadTextureBlock, so the data must remain as-is.
            // Decomp code that reads this data directly as u16* (e.g. font
            // system in print.c print_setBoldFontTexturePixel) must do a BE byte-read instead.
            offset += chunkData.textureData.size();
        }

        // SPDLOG_DEBUG("  Frame buffer size: {} bytes, pointer: {}",
        //             frameSizes[frameIdx], static_cast<void*>(sprite->frames[frameIdx]));
    }
}

BKSprite* Sprite::GetPointer() {
    if (!mSpriteHeader) {
        // SPDLOG_INFO("Building sprite structure on first GetPointer() call");
        BuildSpriteStructure();
    }
    BKSprite* sprite = reinterpret_cast<BKSprite*>(mSpriteHeader.get());
    // SPDLOG_INFO("GetPointer() returning sprite at {} with {} frames",
    //             static_cast<void*>(sprite), sprite->frameCnt);
    if (sprite->frameCnt > 0) {
        // SPDLOG_INFO("  Frame[0] pointer: {}", static_cast<void*>(sprite->frames[0]));
    }
    return sprite;
}

size_t Sprite::GetPointerSize() {
    // Size was calculated during BuildSpriteStructure
    // For now, we don't store it separately, but GetPointer ensures it's built
    if (!mSpriteHeader) {
        BuildSpriteStructure();
    }
    // Return a nominal value since the actual size is managed internally
    return sizeof(BKSprite) + frameCount * sizeof(BKSpriteFrame*);
}

} // namespace Factories