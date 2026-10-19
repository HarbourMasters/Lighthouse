#include "MapFactory.h"

#include "spdlog/spdlog.h"
#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>
#include "factories/bk64/MapFactory.h"

#include "BlobWriter.h"

namespace Factories {
namespace {
void WriteNodeProp(std::vector<uint8_t>& out, const BK64::NodeProp& node) {
    AppendValue<int16_t>(out, node.position[0]);
    AppendValue<int16_t>(out, node.position[1]);
    AppendValue<int16_t>(out, node.position[2]);

    // NodeProp declares `u16 radius:9; bit6:6; bit0:1;`
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // BE: first field at MSB → radius at bits 15-7, bit6 at bits 6-1, bit0 at bit 0
    const uint16_t f1 =
        static_cast<uint16_t>(((node.radius & 0x1FF) << 7) | ((node.bit6 & 0x3F) << 1) | (node.bit0 & 0x01));
#else
    // LE: first field at LSB → radius at bits 0-8, bit6 at bits 9-14, bit0 at bit 15
    const uint16_t f1 =
        static_cast<uint16_t>((node.radius & 0x1FF) | ((node.bit6 & 0x3F) << 9) | ((node.bit0 & 0x01) << 15));
#endif
    AppendValue<uint16_t>(out, f1);
    AppendValue<uint16_t>(out, node.unk8);
    AppendValue<uint8_t>(out, node.unkA);
    AppendValue<uint8_t>(out, node.padB);

    // `u32 yaw:9; scale:23;`
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // BE: yaw at bits 31-23, scale at bits 22-0
    const uint32_t f2 = ((node.yaw & 0x1FF) << 23) | (node.scale & 0x7FFFFF);
#else
    // LE: yaw at bits 0-8, scale at bits 9-31
    const uint32_t f2 = (node.yaw & 0x1FF) | ((node.scale & 0x7FFFFF) << 9);
#endif
    AppendValue<uint32_t>(out, f2);

    // `u32 unk10_31:12; unk10_19:12; pad10_7:1; unk10_6:1; pad10_5:4; unk10_0:2;`
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // BE: first field at MSB → unk10_31 at bits 31-20, unk10_19 at bits 19-8, etc.
    const uint32_t f3 = ((node.unk10_31 & 0xFFF) << 20) | ((node.unk10_19 & 0xFFF) << 8) |
                        ((node.pad10_7 & 0x01) << 7) | ((node.unk10_6 & 0x01) << 6) | ((node.pad10_5 & 0x0F) << 2) |
                        (node.unk10_0 & 0x03);
#else
    // LE: first field at LSB → unk10_31 at bits 0-11, unk10_19 at bits 12-23, etc.
    const uint32_t f3 = (node.unk10_31 & 0xFFF) | ((node.unk10_19 & 0xFFF) << 12) | ((node.pad10_7 & 0x01) << 24) |
                        ((node.unk10_6 & 0x01) << 25) | ((node.pad10_5 & 0x0F) << 26) | ((node.unk10_0 & 0x03) << 30);
#endif
    AppendValue<uint32_t>(out, f3);
}

void WriteFloats(std::vector<uint8_t>& out, uint8_t marker, const float* values, int count) {
    out.push_back(marker);
    for (int i = 0; i < count; i++) {
        AppendValue<float>(out, values[i]);
    }
}
} // namespace

std::vector<uint8_t> LegacySetupBytes(const BK64::MapData& map) {
    std::vector<uint8_t> out;
    out.push_back(0x01);
    out.push_back(0x01);
    for (const int32_t bound : map.mCubeMin) {
        AppendValue<int32_t>(out, bound);
    }
    for (const int32_t bound : map.mCubeMax) {
        AppendValue<int32_t>(out, bound);
    }

    for (const auto& cube : map.mCubes) {
        if (!cube.nodeProps.empty() || !cube.props.empty()) {
            out.push_back(0x03);

            if (!cube.nodeProps.empty()) {
                // [port] The count is one byte, so only emit what it can advertise;
                // writing every entry would desync the reader on an oversized cube.
                const size_t nodeCnt = std::min<size_t>(cube.nodeProps.size(), 0xFF);
                out.push_back(0x0A);
                out.push_back(static_cast<uint8_t>(nodeCnt));
                out.push_back(0x0B);
                for (size_t n = 0; n < nodeCnt; n++) {
                    WriteNodeProp(out, cube.nodeProps[n]);
                }
            }

            if (!cube.props.empty()) {
                const size_t propCnt = std::min<size_t>(cube.props.size(), 0xFF);
                out.push_back(0x08);
                out.push_back(static_cast<uint8_t>(propCnt));
                out.push_back(0x09);
                for (size_t pi = 0; pi < propCnt; pi++) {
                    // [port] Props are raw 12-byte N64 big-endian structs from the ROM.
                    // Layout: u32(4) + s16(2) + s16(2) + s16(2) + u16(2) = 12 bytes.
                    // Byte-swap each multi-byte field to native (little-endian) order
                    // so the decomp code can read them as native structs.
                    const uint8_t* p = cube.props[pi].raw;
                    uint32_t word0 = (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
                    int16_t x = (int16_t)((uint16_t)p[4] << 8 | p[5]);
                    int16_t y = (int16_t)((uint16_t)p[6] << 8 | p[7]);
                    int16_t z = (int16_t)((uint16_t)p[8] << 8 | p[9]);
                    uint16_t flags = (uint16_t)p[10] << 8 | p[11];
                    AppendValue<uint32_t>(out, word0);
                    AppendValue<int16_t>(out, x);
                    AppendValue<int16_t>(out, y);
                    AppendValue<int16_t>(out, z);
                    AppendValue<uint16_t>(out, flags);
                }
            }
        }

        out.push_back(0x01);
    }
    out.push_back(0x00);

    out.push_back(0x03);
    for (const auto& cam : map.mCameraNodes) {
        out.push_back(0x01);
        AppendValue<int16_t>(out, cam.index);

        out.push_back(0x02);
        out.push_back(cam.type);

        switch (cam.type) {
            case 1:
                WriteFloats(out, 0x01, cam.data.type1.position, 3);
                out.push_back(0x02);
                AppendValue<float>(out, cam.data.type1.horizontalSpeed);
                AppendValue<float>(out, cam.data.type1.verticalSpeed);
                out.push_back(0x03);
                AppendValue<float>(out, cam.data.type1.rotation);
                AppendValue<float>(out, cam.data.type1.accelaration);
                WriteFloats(out, 0x04, cam.data.type1.pitchYawRoll, 3);
                out.push_back(0x05);
                AppendValue<int32_t>(out, cam.data.type1.unknownFlag);
                break;
            case 2:
                WriteFloats(out, 0x01, cam.data.type2.position, 3);
                WriteFloats(out, 0x02, cam.data.type2.pitchYawRoll, 3);
                break;
            case 3:
                WriteFloats(out, 0x01, cam.data.type3.position, 3);
                out.push_back(0x02);
                AppendValue<float>(out, cam.data.type3.horizontalSpeed);
                AppendValue<float>(out, cam.data.type3.verticalSpeed);
                out.push_back(0x03);
                AppendValue<float>(out, cam.data.type3.rotation);
                AppendValue<float>(out, cam.data.type3.accelaration);
                out.push_back(0x06);
                AppendValue<float>(out, cam.data.type3.closeDistance);
                AppendValue<float>(out, cam.data.type3.farDistance);
                WriteFloats(out, 0x04, cam.data.type3.pitchYawRoll, 3);
                out.push_back(0x05);
                AppendValue<int32_t>(out, cam.data.type3.unknownFlag);
                break;
            case 4:
                out.push_back(0x01);
                AppendValue<int32_t>(out, cam.data.type4.unknownFlag);
                break;
            case 0:
            default:
                break;
        }

        // Types 1-4 each have a while(!isNextByte(0)) loop that consumes this terminator.
        if (cam.type != 0) {
            out.push_back(0x00);
        }
    }
    out.push_back(0x00);

    out.push_back(0x04);
    for (const auto& light : map.mLightingVectors) {
        out.push_back(0x01);
        WriteFloats(out, 0x02, light.position, 3);
        WriteFloats(out, 0x03, light.fadeRadii, 2);
        out.push_back(0x04);
        AppendValue<int32_t>(out, light.rgb[0]);
        AppendValue<int32_t>(out, light.rgb[1]);
        AppendValue<int32_t>(out, light.rgb[2]);
    }
    out.push_back(0x00);

    out.push_back(0x00);
    return out;
}

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKMapV0::ReadResource(std::shared_ptr<Ship::File> file,
                                           std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    const auto map = BK64::ReadMapResource(std::vector<uint8_t>(file->Buffer->begin(), file->Buffer->end()));
    if (!map) {
        return nullptr;
    }
    // [port] Lighthouse- Hybrid adapter: keep normalized BKMap import, emit legacy chunked bytes for decomp runtime
    // readers.
    return MakeBlob(initData, LegacySetupBytes(*map));
}
} // namespace Factories
