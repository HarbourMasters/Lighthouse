#include "MapFactory.h"

#include "spdlog/spdlog.h"
#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>
#include <array>

namespace Factories {
namespace {
template <typename T> void AppendValue(std::vector<uint8_t>& dst, const T& value) {
    const auto base = dst.size();
    dst.resize(base + sizeof(T));
    std::memcpy(dst.data() + base, &value, sizeof(T));
}

std::shared_ptr<Ship::Blob> MakeBlob(const std::shared_ptr<Ship::ResourceInitData>& initData,
                                     std::vector<uint8_t>&& data) {
    auto blob = std::make_shared<Ship::Blob>(initData);
    blob->Data = std::move(data);
    return blob;
}

struct BKMapNodeProp {
    int16_t position[3];
    uint16_t radius;
    uint8_t bit6;
    uint8_t bit0;
    uint16_t unk8;
    uint8_t unkA;
    uint8_t padB;
    uint32_t yaw;
    uint32_t scale;
    uint32_t unk10_31;
    uint32_t unk10_19;
    uint32_t pad10_7;
    uint32_t unk10_6;
    uint32_t pad10_5;
    uint32_t unk10_0;
};

struct BKMapCube {
    int32_t x;
    int32_t y;
    int32_t z;
    std::vector<BKMapNodeProp> nodeProps;
    std::vector<std::array<uint8_t, 12>> props;
};

struct BKMapCameraNode {
    int16_t index;
    uint8_t type;

    float position[3] = {};
    float horizontalSpeed = 0.0f;
    float verticalSpeed = 0.0f;
    float rotation = 0.0f;
    float accelaration = 0.0f;
    float closeDistance = 0.0f;
    float farDistance = 0.0f;
    float pitchYawRoll[3] = {};
    int32_t unknownFlag = 0;
};

struct BKMapLight {
    float position[3];
    float fadeRadii[2];
    int32_t rgb[3];
};

void WriteNodeProp(std::vector<uint8_t>& out, const BKMapNodeProp& node) {
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

void SerializeLegacyMapData(std::vector<uint8_t>& out, const std::vector<BKMapCube>& cubes, const int32_t minCube[3],
                            const int32_t maxCube[3], const std::vector<BKMapCameraNode>& cameras,
                            const std::vector<BKMapLight>& lights) {
    out.push_back(0x01);
    out.push_back(0x01);
    AppendValue<int32_t>(out, minCube[0]);
    AppendValue<int32_t>(out, minCube[1]);
    AppendValue<int32_t>(out, minCube[2]);
    AppendValue<int32_t>(out, maxCube[0]);
    AppendValue<int32_t>(out, maxCube[1]);
    AppendValue<int32_t>(out, maxCube[2]);

    for (const auto& cube : cubes) {
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
                    const auto& prop = cube.props[pi];
                    // [port] Props are raw 12-byte N64 big-endian structs from the ROM.
                    // Layout: u32(4) + s16(2) + s16(2) + s16(2) + u16(2) = 12 bytes.
                    // Byte-swap each multi-byte field to native (little-endian) order
                    // so the decomp code can read them as native structs.
                    const auto* p = prop.data();
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
    for (const auto& cam : cameras) {
        out.push_back(0x01);
        AppendValue<int16_t>(out, cam.index);

        out.push_back(0x02);
        out.push_back(cam.type);

        switch (cam.type) {
            case 1:
                out.push_back(0x01);
                AppendValue<float>(out, cam.position[0]);
                AppendValue<float>(out, cam.position[1]);
                AppendValue<float>(out, cam.position[2]);
                out.push_back(0x02);
                AppendValue<float>(out, cam.horizontalSpeed);
                AppendValue<float>(out, cam.verticalSpeed);
                out.push_back(0x03);
                AppendValue<float>(out, cam.rotation);
                AppendValue<float>(out, cam.accelaration);
                out.push_back(0x04);
                AppendValue<float>(out, cam.pitchYawRoll[0]);
                AppendValue<float>(out, cam.pitchYawRoll[1]);
                AppendValue<float>(out, cam.pitchYawRoll[2]);
                out.push_back(0x05);
                AppendValue<int32_t>(out, cam.unknownFlag);
                break;
            case 2:
                out.push_back(0x01);
                AppendValue<float>(out, cam.position[0]);
                AppendValue<float>(out, cam.position[1]);
                AppendValue<float>(out, cam.position[2]);
                out.push_back(0x02);
                AppendValue<float>(out, cam.pitchYawRoll[0]);
                AppendValue<float>(out, cam.pitchYawRoll[1]);
                AppendValue<float>(out, cam.pitchYawRoll[2]);
                break;
            case 3:
                out.push_back(0x01);
                AppendValue<float>(out, cam.position[0]);
                AppendValue<float>(out, cam.position[1]);
                AppendValue<float>(out, cam.position[2]);
                out.push_back(0x02);
                AppendValue<float>(out, cam.horizontalSpeed);
                AppendValue<float>(out, cam.verticalSpeed);
                out.push_back(0x03);
                AppendValue<float>(out, cam.rotation);
                AppendValue<float>(out, cam.accelaration);
                out.push_back(0x06);
                AppendValue<float>(out, cam.closeDistance);
                AppendValue<float>(out, cam.farDistance);
                out.push_back(0x04);
                AppendValue<float>(out, cam.pitchYawRoll[0]);
                AppendValue<float>(out, cam.pitchYawRoll[1]);
                AppendValue<float>(out, cam.pitchYawRoll[2]);
                out.push_back(0x05);
                AppendValue<int32_t>(out, cam.unknownFlag);
                break;
            case 4:
                out.push_back(0x01);
                AppendValue<int32_t>(out, cam.unknownFlag);
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
    for (const auto& light : lights) {
        out.push_back(0x01);
        out.push_back(0x02);
        AppendValue<float>(out, light.position[0]);
        AppendValue<float>(out, light.position[1]);
        AppendValue<float>(out, light.position[2]);
        out.push_back(0x03);
        AppendValue<float>(out, light.fadeRadii[0]);
        AppendValue<float>(out, light.fadeRadii[1]);
        out.push_back(0x04);
        AppendValue<int32_t>(out, light.rgb[0]);
        AppendValue<int32_t>(out, light.rgb[1]);
        AppendValue<int32_t>(out, light.rgb[2]);
    }
    out.push_back(0x00);

    out.push_back(0x00);
}
} // namespace

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKMapV0::ReadResource(std::shared_ptr<Ship::File> file,
                                           std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);
    const auto cubeCount = reader->ReadUInt32();
    int32_t minCube[3] = { reader->ReadInt32(), reader->ReadInt32(), reader->ReadInt32() };
    int32_t maxCube[3] = { reader->ReadInt32(), reader->ReadInt32(), reader->ReadInt32() };

    auto cubes = std::vector<BKMapCube>();
    cubes.reserve(cubeCount);
    for (uint32_t i = 0; i < cubeCount; i++) {
        BKMapCube cube;
        const auto cubeHeader = reader->ReadUInt32();
        cube.x = static_cast<int32_t>((cubeHeader >> 27) & 0x1F);
        cube.y = static_cast<int32_t>((cubeHeader >> 22) & 0x1F);
        cube.z = static_cast<int32_t>((cubeHeader >> 17) & 0x1F);

        const auto nodeCount = reader->ReadUInt32();
        cube.nodeProps.reserve(nodeCount);
        for (uint32_t n = 0; n < nodeCount; n++) {
            BKMapNodeProp node{};
            // [port] Read individual typed fields directly (no BE bit extraction)
            // Matches Torch's native-endian field-by-field export format
            node.position[0] = reader->ReadInt16();
            node.position[1] = reader->ReadInt16();
            node.position[2] = reader->ReadInt16();
            node.radius = reader->ReadUInt16();
            node.bit6 = reader->ReadUByte();
            node.bit0 = reader->ReadUByte();
            node.unk8 = reader->ReadUInt16();
            node.unkA = reader->ReadUByte();
            node.padB = reader->ReadUByte();
            node.yaw = reader->ReadUInt16();
            node.scale = reader->ReadUInt32();
            node.unk10_31 = reader->ReadUInt16();
            node.unk10_19 = reader->ReadUInt16();
            node.pad10_7 = reader->ReadUByte();
            node.unk10_6 = reader->ReadUByte();
            node.pad10_5 = reader->ReadUByte();
            node.unk10_0 = reader->ReadUByte();
            cube.nodeProps.push_back(node);
        }

        const auto propCount = reader->ReadUInt32();
        cube.props.reserve(propCount);
        for (uint32_t p = 0; p < propCount; p++) {
            std::array<uint8_t, 12> prop{};
            reader->Read(reinterpret_cast<char*>(prop.data()), static_cast<int32_t>(prop.size()));
            cube.props.push_back(prop);
        }

        cubes.push_back(cube);
    }

    const auto cameraCount = reader->ReadUInt32();
    auto cameras = std::vector<BKMapCameraNode>();
    cameras.reserve(cameraCount);
    for (uint32_t i = 0; i < cameraCount; i++) {
        BKMapCameraNode cam{};
        cam.index = reader->ReadInt16();
        cam.type = reader->ReadUByte();

        switch (cam.type) {
            case 1:
                cam.position[0] = reader->ReadFloat();
                cam.position[1] = reader->ReadFloat();
                cam.position[2] = reader->ReadFloat();
                cam.horizontalSpeed = reader->ReadFloat();
                cam.verticalSpeed = reader->ReadFloat();
                cam.rotation = reader->ReadFloat();
                cam.accelaration = reader->ReadFloat();
                cam.pitchYawRoll[0] = reader->ReadFloat();
                cam.pitchYawRoll[1] = reader->ReadFloat();
                cam.pitchYawRoll[2] = reader->ReadFloat();
                cam.unknownFlag = reader->ReadInt32();
                break;
            case 2:
                cam.position[0] = reader->ReadFloat();
                cam.position[1] = reader->ReadFloat();
                cam.position[2] = reader->ReadFloat();
                cam.pitchYawRoll[0] = reader->ReadFloat();
                cam.pitchYawRoll[1] = reader->ReadFloat();
                cam.pitchYawRoll[2] = reader->ReadFloat();
                break;
            case 3:
                cam.position[0] = reader->ReadFloat();
                cam.position[1] = reader->ReadFloat();
                cam.position[2] = reader->ReadFloat();
                cam.horizontalSpeed = reader->ReadFloat();
                cam.verticalSpeed = reader->ReadFloat();
                cam.rotation = reader->ReadFloat();
                cam.accelaration = reader->ReadFloat();
                cam.closeDistance = reader->ReadFloat();
                cam.farDistance = reader->ReadFloat();
                cam.pitchYawRoll[0] = reader->ReadFloat();
                cam.pitchYawRoll[1] = reader->ReadFloat();
                cam.pitchYawRoll[2] = reader->ReadFloat();
                cam.unknownFlag = reader->ReadInt32();
                break;
            case 4:
                cam.unknownFlag = reader->ReadInt32();
                break;
            case 0:
            default:
                break;
        }

        cameras.push_back(cam);
    }

    const auto lightCount = reader->ReadUInt32();
    auto lights = std::vector<BKMapLight>();
    lights.reserve(lightCount);
    for (uint32_t i = 0; i < lightCount; i++) {
        BKMapLight light{};
        light.position[0] = reader->ReadFloat();
        light.position[1] = reader->ReadFloat();
        light.position[2] = reader->ReadFloat();
        light.fadeRadii[0] = reader->ReadFloat();
        light.fadeRadii[1] = reader->ReadFloat();
        light.rgb[0] = reader->ReadInt32();
        light.rgb[1] = reader->ReadInt32();
        light.rgb[2] = reader->ReadInt32();
        lights.push_back(light);
    }

    // [port] Lighthouse- Hybrid adapter: keep normalized BKMap import, emit legacy chunked bytes for decomp runtime
    // readers.
    auto out = std::vector<uint8_t>();
    SerializeLegacyMapData(out, cubes, minCube, maxCube, cameras, lights);
    return MakeBlob(initData, std::move(out));
}
} // namespace Factories
