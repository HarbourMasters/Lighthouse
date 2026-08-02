#ifndef NETWORK_ANCHOR_JSON_CONVERSIONS_H
#define NETWORK_ANCHOR_JSON_CONVERSIONS_H
#ifdef __cplusplus

#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "Anchor.h"

extern "C" {
//#include "z64.h"
}

using json = nlohmann::json;

inline void from_json(const json& j, Color_RGB8& color) {
    j.at("r").get_to(color.r);
    j.at("g").get_to(color.g);
    j.at("b").get_to(color.b);
}

inline void to_json(json& j, const Color_RGB8& color) {
    j = json{ { "r", color.r }, { "g", color.g }, { "b", color.b } };
}

// Model colors travel as a fixed-length array, one entry per BKColorChannel. Clients that
// predate the feature simply omit the key and everyone renders them with vanilla colors.
inline void from_json(const json& j, BKPlayerColorSet& colors) {
    colors = BKPlayerColorSet{};
    if (!j.is_array()) {
        return;
    }
    for (size_t i = 0; i < j.size() && i < (size_t)BK_COLOR_CHANNEL_COUNT; i++) {
        const json& entry = j[i];
        if (!entry.is_object()) {
            continue;
        }
        colors.channel[i].enabled = entry.value("on", false) ? 1 : 0;
        colors.channel[i].r = entry.value("r", (u8)0);
        colors.channel[i].g = entry.value("g", (u8)0);
        colors.channel[i].b = entry.value("b", (u8)0);
    }
}

inline void to_json(json& j, const BKPlayerColorSet& colors) {
    j = json::array();
    for (int i = 0; i < BK_COLOR_CHANNEL_COUNT; i++) {
        j.push_back(json{ { "on", colors.channel[i].enabled != 0 },
                          { "r", colors.channel[i].r },
                          { "g", colors.channel[i].g },
                          { "b", colors.channel[i].b } });
    }
}

inline void from_json(const json& j, AnchorClient& client) {
    client.clientId = j.value("clientId", (u32)0);
    client.name = j.value("name", "???");
    client.colors = BKPlayerColorSet{};
    if (j.contains("colors")) {
        from_json(j.at("colors"), client.colors);
    }
    client.clientVersion = j.value("clientVersion", "???");
    client.teamId = j.value("teamId", "default");
    client.online = j.value("online", false);
    client.seed = j.value("seed", (u32)0);
    client.isSaveLoaded = j.value("isSaveLoaded", false);
    client.isGameComplete = j.value("isGameComplete", false);
    client.map = j.value("map", MAP_0_UNKNOWN);
    client.exit = j.value("exit", (s32)0);
    client.self = j.value("self", false);
    client.cutsceneReturnMap = 0;
}

#endif // __cplusplus
#endif // NETWORK_ANCHOR_JSON_CONVERSIONS_H
