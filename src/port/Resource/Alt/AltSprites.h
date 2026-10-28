#pragma once

namespace BkSpriteAlt {

const char* RegisteredChunkPath(const void* chunkAddr);

} // namespace BkSpriteAlt

extern "C" {
void AltSprites_RegisterChunk(const void* chunkAddr, const char* path);
void AltSprites_UnregisterChunk(const void* chunkAddr);
}
