#include "OcclusionDebug.h"

#include <map>
#include <set>
#include <mutex>
#include <vector>
#include <string>

#include <imgui.h>
#include <libultraship/bridge.h>
#include <ship/Context.h>
#include <spdlog/spdlog.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Patches/GeoCull.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h" // gsworld_getMap, mapModel_getModelBin

// mapModel_getModelBin returns BKModelBin*; the debugger only compares it as an opaque
// pointer, so reinterpret to const void* at the call sites.

#define CVAR_OCCLUSION_ACTIVE CVAR_DEVELOPER_TOOLS("OcclusionDebug.Active")

namespace {

constexpr int kParts = 3; // 0 = opaque map model, 1 = translucent map model, 2 = other
const char* kPartNames[kParts] = { "OPA", "XLU", "?" };

const char* CmdTypeName(int type) {
    switch (type) {
        case OCCLUSION_CMD_CAMERA:
            return "CAMERA";
        case OCCLUSION_CMD_LOD:
            return "LOD";
        case OCCLUSION_CMD_UNKE:
            return "UnkE";
        default:
            return "?";
    }
}

// Each command is keyed by (model part, byte offset within the model's geo command list).
struct Key {
    int part;
    int offset;
    bool operator<(const Key& o) const {
        return (part != o.part) ? (part < o.part) : (offset < o.offset);
    }
};

struct CullCmdRecord {
    int type;
    int areaCount;
    unsigned char areaIds[12]; // CAMERA only
    int detail0;               // CAMERA: flags (unkB); LOD: min distance
    int detail1;               // LOD: max distance
    int drawnVanilla;
    bool seenThisFrame;
};

std::mutex sMutex;
std::map<Key, CullCmdRecord> sRecorded; // accumulated across frames, cleared on map change
std::set<Key> sForceDraw;
bool sForceAll = false;
int sRecordedMap = -1;

int PartForBin(const void* bin) {
    if (bin == (const void*)mapModel_getModelBin(0)) {
        return 0;
    }
    if (bin == (const void*)mapModel_getModelBin(1)) {
        return 1;
    }
    return 2;
}

std::string DetailString(const CullCmdRecord& rec) {
    if (rec.type == OCCLUSION_CMD_CAMERA) {
        std::string ids;
        for (int i = 0; i < rec.areaCount; i++) {
            if (i) {
                ids += ",";
            }
            ids += std::to_string((int)rec.areaIds[i]);
        }
        const char* branch =
            (rec.detail0 & 1) ? ((rec.detail0 & 2) ? "in+out" : "outside") : ((rec.detail0 & 2) ? "inside" : "-");
        return "areas {" + ids + "} " + branch;
    }
    if (rec.type == OCCLUSION_CMD_LOD) {
        return "dist " + std::to_string(rec.detail0) + ".." + std::to_string(rec.detail1);
    }
    if (rec.type == OCCLUSION_CMD_UNKE) {
        return "frustum sphere";
    }
    return "";
}

// OnGeoCull listener: record the command and apply the developer's force-draw choices.
void OnGeoCull_Record(IEvent* event) {
    auto* ev = reinterpret_cast<OnGeoCull*>(event);

    Key key{ PartForBin(ev->modelBin), ev->offset };

    std::lock_guard<std::mutex> lock(sMutex);
    CullCmdRecord& rec = sRecorded[key];
    rec.type = ev->type;
    rec.areaCount = (ev->areaCount > 12) ? 12 : (ev->areaCount < 0 ? 0 : ev->areaCount);
    for (int i = 0; i < 12; i++) {
        rec.areaIds[i] = (i < rec.areaCount && ev->areaIds != nullptr) ? ev->areaIds[i] : 0;
    }
    rec.detail0 = ev->detail0;
    rec.detail1 = ev->detail1;
    rec.drawnVanilla = ev->drawnVanilla;
    rec.seenThisFrame = true;

    if (sForceAll || sForceDraw.count(key)) {
        *ev->forceDraw = true;
    }
}

// Per game tick: clear "seen this frame" flags and reset on map change.
void OnFrame_Reset(IEvent*) {
    std::lock_guard<std::mutex> lock(sMutex);
    int map = (int)gsworld_getMap();
    if (map != sRecordedMap) {
        sRecorded.clear();
        sForceDraw.clear();
        sRecordedMap = map;
    }
    for (auto& [key, rec] : sRecorded) {
        rec.seenThisFrame = false;
    }
}

} // namespace

void RegisterOcclusionDebug_Init() {
    bool active = CVarGetInteger(CVAR_OCCLUSION_ACTIVE, 0);
    GeoCull_SetConsumer(GEOCULL_CONSUMER_DEBUG, active);
    COND_HOOK(OnGeoCull, EVENT_PRIORITY_NORMAL, active, OnGeoCull_Record);
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, active, OnFrame_Reset);
}

static RegisterShipInitFunc sInitOcclusionDebug(RegisterOcclusionDebug_Init, { CVAR_OCCLUSION_ACTIVE });

void OcclusionDebugWindow::DrawElement() {
    bool active = CVarGetInteger(CVAR_OCCLUSION_ACTIVE, 0);
    if (ImGui::Checkbox("Enable recording / force-draw", &active)) {
        CVarSetInteger(CVAR_OCCLUSION_ACTIVE, active);
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
    ImGui::TextWrapped(
        "Flip 'Force draw all' on for a moment to walk every cull command and populate the list (the "
        "screen will look broken - that is expected), then turn it off and tick 'Force Draw' on "
        "individual rows to find the chunk(s) that reveal the target scenery. Offsets are stable, so the "
        "list never shifts. Dump the chosen set to the log when done.");

    struct Row {
        Key key;
        CullCmdRecord rec;
        bool forced;
    };
    std::vector<Row> rows;
    bool forceAll;
    int mapId;
    {
        std::lock_guard<std::mutex> lock(sMutex);
        forceAll = sForceAll;
        mapId = sRecordedMap;
        rows.reserve(sRecorded.size());
        for (auto& [key, rec] : sRecorded) {
            rows.push_back({ key, rec, sForceDraw.count(key) > 0 });
        }
    }

    ImGui::Separator();
    ImGui::Text("Map: 0x%X    Cull commands seen: %zu", mapId, rows.size());

    if (ImGui::Checkbox("Force draw all (enumerate)", &forceAll)) {
        std::lock_guard<std::mutex> lock(sMutex);
        sForceAll = forceAll;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear list")) {
        std::lock_guard<std::mutex> lock(sMutex);
        sRecorded.clear();
        sRecordedMap = -1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear force-draw")) {
        std::lock_guard<std::mutex> lock(sMutex);
        sForceDraw.clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Dump force-draw set to log")) {
        std::string parts[kParts];
        {
            std::lock_guard<std::mutex> lock(sMutex);
            for (const Key& k : sForceDraw) {
                if (k.part < 0 || k.part >= kParts) {
                    continue;
                }
                char buf[16];
                snprintf(buf, sizeof(buf), "0x%X", k.offset);
                if (!parts[k.part].empty()) {
                    parts[k.part] += ", ";
                }
                parts[k.part] += buf;
            }
        }
        SPDLOG_INFO("[OcclusionDebug] map 0x{:X}: OPA {{{}}} XLU {{{}}}", mapId, parts[0], parts[1]);
    }

    if (ImGui::BeginTable("OcclusionCmds", 6,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit |
                              ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Part", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Detail", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Drawn", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Force Draw", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (Row& row : rows) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%s", kPartNames[row.key.part]);

            ImGui::TableNextColumn();
            ImGui::Text("0x%X", row.key.offset);

            ImGui::TableNextColumn();
            ImGui::Text("%s", CmdTypeName(row.rec.type));

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", DetailString(row.rec).c_str());

            ImGui::TableNextColumn();
            if (!row.rec.seenThisFrame) {
                ImGui::TextDisabled("--");
            } else {
                ImGui::Text("%s", row.rec.drawnVanilla ? "yes" : "no");
            }

            ImGui::TableNextColumn();
            bool fd = row.forced;
            std::string id = "##fd" + std::to_string(row.key.part) + "_" + std::to_string(row.key.offset);
            if (ImGui::Checkbox(id.c_str(), &fd)) {
                std::lock_guard<std::mutex> lock(sMutex);
                if (fd) {
                    sForceDraw.insert(row.key);
                } else {
                    sForceDraw.erase(row.key);
                }
            }
        }
        ImGui::EndTable();
    }
}
