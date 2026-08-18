#include "CameraTools.h"

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Patches/Patches.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/UIWidgets.hpp"

#include <cstdio>
#include <map>
#include <utility>
#include <imgui.h>
#include <libultraship/libultraship.h>

#include "enums.h"
#include "functions.h"
#include "variables.h"

static constexpr s32 kPivotCameraNodeType = 1;
static constexpr s32 kStaticCameraNodeType = 2;
static constexpr s32 kZoomCameraNodeType = 3;
static const char* kCameraNodeTypeNames[] = { "Unknown", "Pivot", "Static", "Zoom", "Random" };

struct CameraNodeInfo {
    s32 type = 0;
    bool hasTransform = false;
    f32 position[3] = { 0.0f, 0.0f, 0.0f };
    f32 rotation[3] = { 0.0f, 0.0f, 0.0f };
};

struct ActiveCamera {
    bool valid = false;
    s32 source = CAMERA_TYPE_3_STATIC;
    s32 id = -1;
    f32 position[3] = { 0.0f, 0.0f, 0.0f };
    f32 authored[3] = { 0.0f, 0.0f, 0.0f };
    f32 fixed[3] = { 0.0f, 0.0f, 0.0f };
    f32 applied[3] = { 0.0f, 0.0f, 0.0f };
};

struct CameraNudge {
    f32 rotation[3] = { 0.0f, 0.0f, 0.0f };
};

static ActiveCamera sActiveCamera;
static std::map<std::pair<s32, s32>, CameraNudge> sCameraNudges;

static f32 CameraYawDelta(f32 applied, f32 authored) {
    f32 delta = applied - authored;
    while (delta > 180.0f) {
        delta -= 360.0f;
    }
    while (delta < -180.0f) {
        delta += 360.0f;
    }
    return delta;
}

static const char* CameraSourceName(s32 source) {
    return source == CAMERA_TYPE_1_UNKNOWN ? "Cutscene" : "Static node";
}

static const char* CameraSourceEnumName(s32 source) {
    return source == CAMERA_TYPE_1_UNKNOWN ? "CAMERA_TYPE_1_UNKNOWN" : "CAMERA_TYPE_3_STATIC";
}

static const char* CameraNodeTypeName(s32 type) {
    return (type >= 0 && type <= 4) ? kCameraNodeTypeNames[type] : "?";
}

static const char* CameraTypeName(s32 type) {
    switch (type) {
        case CAMERA_TYPE_1_UNKNOWN:
            return "Unknown(1)";
        case CAMERA_TYPE_2_DYNAMIC:
            return "Dynamic";
        case CAMERA_TYPE_3_STATIC:
            return "Static";
        case CAMERA_TYPE_4_RANDOM:
            return "Random";
        default:
            return "None";
    }
}

static bool ReadCameraNode(s32 node, CameraNodeInfo& out) {
    if (!ncCameraNodeList_nodeIsValid(node)) {
        return false;
    }
    out = CameraNodeInfo{};
    out.type = ncCameraNodeList_getNodeType(node);

    const f32* position = nullptr;
    const f32* rotation = nullptr;
    switch (out.type) {
        case kPivotCameraNodeType:
            if (PivotCameraNode* data = ncCameraNodeList_getPivotCameraNode(node)) {
                position = data->position;
                rotation = data->pitchYawRoll;
            }
            break;
        case kStaticCameraNodeType:
            if (StaticCameraNode* data = ncCameraNodeList_getStaticCameraNode(node)) {
                position = data->position;
                rotation = data->pitchYawRoll;
            }
            break;
        case kZoomCameraNodeType:
            if (ZoomCameraNode* data = ncCameraNodeList_getZoomCameraNode(node)) {
                position = data->position;
                rotation = data->pitchYawRoll;
            }
            break;
        default:
            break;
    }

    if (position != nullptr && rotation != nullptr) {
        for (int i = 0; i < 3; i++) {
            out.position[i] = position[i];
            out.rotation[i] = rotation[i];
        }
        out.hasTransform = true;
    }
    return true;
}

static void FormatWsCameraFixRow(char* out, size_t size, s32 map, const ActiveCamera& cam, const f32 adjust[3]) {
    if (cam.source == CAMERA_TYPE_1_UNKNOWN) {
        snprintf(out, size,
                 "{ .map = 0x%X, .source = %s, .matchTransform = true, .position = { %.2ff, %.2ff, %.2ff }, "
                 ".rotation = { %.2ff, %.2ff, %.2ff }, .adjust = { %.1ff, %.1ff, %.1ff } },",
                 map, CameraSourceEnumName(cam.source), cam.position[0], cam.position[1], cam.position[2],
                 cam.authored[0], cam.authored[1], cam.authored[2], adjust[0], adjust[1], adjust[2]);
    } else {
        snprintf(out, size, "{ .map = 0x%X, .source = %s, .id = 0x%02X, .adjust = { %.1ff, %.1ff, %.1ff } },", map,
                 CameraSourceEnumName(cam.source), cam.id, adjust[0], adjust[1], adjust[2]);
    }
}

void CameraTools_Register() {
    REGISTER_LISTENER(CameraRotationAuthored, EVENT_PRIORITY_LOW, [](IEvent* event) {
        auto* ev = (CameraRotationAuthored*)event;
        sActiveCamera.valid = true;
        sActiveCamera.source = ev->source;
        sActiveCamera.id = ev->id;
        for (int i = 0; i < 3; i++) {
            sActiveCamera.authored[i] = ev->rotation[i];
            if (ev->position != nullptr) {
                sActiveCamera.position[i] = ev->position[i];
            }
        }
    });
    REGISTER_LISTENER(CameraRotationAuthored, EVENT_PRIORITY_HIGH, [](IEvent* event) {
        auto* ev = (CameraRotationAuthored*)event;
        auto nudge = sCameraNudges.find({ ev->source, ev->id });
        for (int i = 0; i < 3; i++) {
            sActiveCamera.fixed[i] = ev->rotation[i];
            if (nudge != sCameraNudges.end()) {
                ev->rotation[i] = mlNormalizeAngle(ev->rotation[i] + nudge->second.rotation[i]);
            }
            sActiveCamera.applied[i] = ev->rotation[i];
        }
    });
    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        sActiveCamera = ActiveCamera{};
        sCameraNudges.clear();
    });
}

void DrawCameraTools() {
    s32 map = gsworld_getMap();
    s32 cameraType = ncCamera_getType();
    bool staticCameraLive = (cameraType == CAMERA_TYPE_3_STATIC);
    f32 livePosition[3];
    f32 liveRotation[3];
    viewport_getPosition_vec3f(livePosition);
    viewport_getRotation_vec3f(liveRotation);
    const f32 scale[3] = { port_wsCameraPitchScale(), port_wsCameraYawScale(), 1.0f };
    f32 delta[3];
    f32 tableAdjust[3];
    for (int i = 0; i < 3; i++) {
        delta[i] = CameraYawDelta(sActiveCamera.applied[i], sActiveCamera.authored[i]);
        tableAdjust[i] = scale[i] > 0.0f ? delta[i] / scale[i] : delta[i];
    }
    char fixRow[320] = "";
    if (sActiveCamera.valid) {
        FormatWsCameraFixRow(fixRow, sizeof(fixRow), map, sActiveCamera, tableAdjust);
    }

    ImGui::SeparatorText("Live Camera");
    ImGui::Text("Map: %s (%d / 0x%X)   Camera: %s", port_mapName(map), map, map, CameraTypeName(cameraType));
    ImGui::Text("Pos %9.2f %9.2f %9.2f   Pitch/Yaw/Roll %7.2f %7.2f %7.2f", livePosition[0], livePosition[1],
                livePosition[2], liveRotation[0], liveRotation[1], liveRotation[2]);

    if (sActiveCamera.valid) {
        ImGui::Text("Source: %s   Id: %d / 0x%02X", CameraSourceName(sActiveCamera.source), sActiveCamera.id,
                    sActiveCamera.id);

        auto key = std::make_pair(sActiveCamera.source, sActiveCamera.id);
        auto stored = sCameraNudges.find(key);
        CameraNudge nudge = stored != sCameraNudges.end() ? stored->second : CameraNudge{};

        ImGui::SetNextItemWidth(240.0f);
        bool edited = ImGui::SliderFloat("Yaw Nudge", &nudge.rotation[1], -45.0f, 45.0f, "%+.1f deg");
        ImGui::SetNextItemWidth(240.0f);
        edited |= ImGui::SliderFloat("Pitch Nudge", &nudge.rotation[0], -45.0f, 45.0f, "%+.1f deg");
        ImGui::SetNextItemWidth(240.0f);
        edited |= ImGui::SliderFloat("Roll Nudge", &nudge.rotation[2], -45.0f, 45.0f, "%+.1f deg");

        if (edited) {
            sCameraNudges[key] = nudge;
            f32 rotation[3];
            for (int i = 0; i < 3; i++) {
                rotation[i] = mlNormalizeAngle(sActiveCamera.fixed[i] + nudge.rotation[i]);
                sActiveCamera.applied[i] = rotation[i];
            }
            if (staticCameraLive) {
                f32 position[3];
                ncStaticCamera_getPosition(position);
                ncStaticCamera_setPositionAndRotation(position, rotation);
            } else {
                viewport_setRotation_vec3f(rotation);
            }
        }

        ImGui::Text("Delta pyr %+.1f %+.1f %+.1f   16:9 pyr %+.1f %+.1f %+.1f (scale %.2fx pitch, %.2fx yaw)", delta[0],
                    delta[1], delta[2], tableAdjust[0], tableAdjust[1], tableAdjust[2], scale[0], scale[1]);
        ImGui::TextDisabled("%s", fixRow);
    } else {
        ImGui::TextDisabled("No camera has authored a rotation yet on this map.");
    }

    ImGui::SeparatorText("Camera Nodes");

    if (UIWidgets::Button("Dump To Log", { .color = THEME_COLOR })) {
        BK_LOG_DEBUG("camera nodes for map %s (%d / 0x%X), camera type %s:", port_mapName(map), map, map,
                     CameraTypeName(cameraType));
        BK_LOG_DEBUG("  live viewport  pos %9.2f %9.2f %9.2f  pyr %7.2f %7.2f %7.2f", livePosition[0], livePosition[1],
                     livePosition[2], liveRotation[0], liveRotation[1], liveRotation[2]);
        if (sActiveCamera.valid) {
            BK_LOG_DEBUG("  active camera  %s id %d (0x%02X)  pos %9.2f %9.2f %9.2f  authored pyr %7.2f %7.2f %7.2f  "
                         "applied %7.2f %7.2f %7.2f  delta %+.2f %+.2f %+.2f",
                         CameraSourceName(sActiveCamera.source), sActiveCamera.id, sActiveCamera.id,
                         sActiveCamera.position[0], sActiveCamera.position[1], sActiveCamera.position[2],
                         sActiveCamera.authored[0], sActiveCamera.authored[1], sActiveCamera.authored[2],
                         sActiveCamera.applied[0], sActiveCamera.applied[1], sActiveCamera.applied[2], delta[0],
                         delta[1], delta[2]);
            if (delta[0] != 0.0f || delta[1] != 0.0f || delta[2] != 0.0f) {
                BK_LOG_DEBUG("    %s", fixRow);
            }
        }
        s32 dumped = 0;
        for (s32 node = 0; node < __ncCameraNodeList_capacity(); node++) {
            CameraNodeInfo info;
            if (!ReadCameraNode(node, info)) {
                continue;
            }
            dumped++;
            if (info.hasTransform) {
                BK_LOG_DEBUG("  node %2d (0x%02X)  %-7s  pos %9.2f %9.2f %9.2f  pyr %7.2f %7.2f %7.2f", node, node,
                             CameraNodeTypeName(info.type), info.position[0], info.position[1], info.position[2],
                             info.rotation[0], info.rotation[1], info.rotation[2]);
            } else {
                BK_LOG_DEBUG("  node %2d (0x%02X)  %-7s  (no transform)", node, node, CameraNodeTypeName(info.type));
            }
        }
        if (dumped == 0) {
            BK_LOG_DEBUG("No cameras found to dump.");
        }
    }

    if (!ImGui::BeginTable("CameraNodeTable", 8,
                           ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        return;
    }
    ImGui::TableSetupColumn("Node");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
    ImGui::TableSetupColumn("Z");
    ImGui::TableSetupColumn("Pitch");
    ImGui::TableSetupColumn("Yaw");
    ImGui::TableSetupColumn("Roll");
    ImGui::TableHeadersRow();

    s32 found = 0;
    for (s32 node = 0; node < __ncCameraNodeList_capacity(); node++) {
        CameraNodeInfo info;
        if (!ReadCameraNode(node, info)) {
            continue;
        }
        found++;
        bool active = staticCameraLive && info.type == kStaticCameraNodeType && sActiveCamera.valid &&
                      sActiveCamera.source == CAMERA_TYPE_3_STATIC && node == sActiveCamera.id;

        ImGui::TableNextRow();
        if (active) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
        }
        ImGui::TableNextColumn();
        ImGui::Text("%d / 0x%02X", node, node);
        ImGui::TableNextColumn();
        ImGui::Text("%s", CameraNodeTypeName(info.type));

        for (int i = 0; i < 3; i++) {
            ImGui::TableNextColumn();
            if (info.hasTransform) {
                ImGui::Text("%.2f", info.position[i]);
            } else {
                ImGui::TextDisabled("-");
            }
        }
        for (int i = 0; i < 3; i++) {
            ImGui::TableNextColumn();
            if (info.hasTransform) {
                ImGui::Text("%.2f", info.rotation[i]);
            } else {
                ImGui::TextDisabled("-");
            }
        }
    }
    ImGui::EndTable();

    if (found == 0) {
        ImGui::TextDisabled("This map has no camera nodes. Use the Live Camera "
                            "editor above.");
    }
}

static RegisterShipInitFunc cameraToolsInitFunc(CameraTools_Register);
