#pragma once

// Template definitions for Scalar<V>. Include this ONLY from a .cpp that instantiates an
// uncommon V — the common instantiations are already emitted by Setting.cpp. Ordinary code
// includes Setting.h, which is deliberately free of the full JSON parser.

#include <algorithm>
#include <type_traits>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "Pref.h"

// Global scope so nlohmann finds these by ADL — Color_RGBA8 is a global typedef.
inline void to_json(nlohmann::json& j, const Color_RGBA8& c) {
    j = nlohmann::json{ { "r", c.r }, { "g", c.g }, { "b", c.b }, { "a", c.a } };
}

inline void from_json(const nlohmann::json& j, Color_RGBA8& c) {
    c.r = j.at("r").get<uint8_t>();
    c.g = j.at("g").get<uint8_t>();
    c.b = j.at("b").get<uint8_t>();
    c.a = j.at("a").get<uint8_t>();
}

namespace Prefs {

inline void to_json(nlohmann::json& j, const ColorValue& c) {
    j = nlohmann::json{ { "value", c.value }, { "rainbow", c.rainbow }, { "locked", c.locked } };
}

inline void from_json(const nlohmann::json& j, ColorValue& c) {
    // A bare {r,g,b,a} object reads as a plain color with no picker state, so a color written
    // before it grew a picker still loads.
    if (j.contains("r")) {
        c.value = j.get<Color_RGBA8>();
        return;
    }
    c.value = j.at("value").get<Color_RGBA8>();
    c.rainbow = j.value("rainbow", false);
    c.locked = j.value("locked", false);
}

template <typename V> bool Scalar<V>::Validate(V& value) const {
    if constexpr (std::is_arithmetic_v<V>) {
        if (mOptions.min.has_value() && value < *mOptions.min) {
            if (!mOptions.clamp) {
                return false;
            }
            value = *mOptions.min;
        }
        if (mOptions.max.has_value() && value > *mOptions.max) {
            if (!mOptions.clamp) {
                return false;
            }
            value = *mOptions.max;
        }
    }

    if constexpr (HasSize<V>) {
        if (mOptions.maxSize != 0 && value.size() > mOptions.maxSize) {
            if (!mOptions.clamp) {
                return false;
            }
            // Only sequence-like types can be truncated in place; anything else is rejected.
            if constexpr (requires(V & v, size_t n) { v.resize(n); }) {
                value.resize(mOptions.maxSize);
            } else {
                return false;
            }
        }
    }

    if (!mOptions.oneOf.empty()) {
        const bool allowed = std::any_of(mOptions.oneOf.begin(), mOptions.oneOf.end(),
                                         [&value](const V& candidate) { return ValueEquals(candidate, value); });
        if (!allowed) {
            return false;
        }
    }

    if (mOptions.validator && !mOptions.validator(value)) {
        return false;
    }

    return true;
}

template <typename V> void Scalar<V>::Write(nlohmann::json& out) const {
    out = mValue;
}

template <typename V> bool Scalar<V>::Read(const nlohmann::json& in) {
    V parsed{};

    if constexpr (std::is_same_v<V, bool>) {
        // nlohmann is asymmetric: json(true).get<int32_t>() succeeds, but json(1).get<bool>() throws.
        // Write always emits true/false, so accept a hand-written 0/1 here rather than discarding it.
        if (in.is_boolean()) {
            parsed = in.get<bool>();
        } else if (in.is_number_integer()) {
            parsed = in.get<int64_t>() != 0;
        } else {
            SPDLOG_WARN("Setting '{}' is not a boolean in the config; using the default", mPath);
            return false;
        }
    } else {
        try {
            parsed = in.get<V>();
        } catch (const std::exception& e) {
            SPDLOG_WARN("Setting '{}' has the wrong type in the config ({}); using the default", mPath, e.what());
            return false;
        }
    }

    if (!Validate(parsed)) {
        SPDLOG_WARN("Setting '{}' failed validation; using the default", mPath);
        return false;
    }

    mValue = std::move(parsed);
    return true;
}

} // namespace Prefs
