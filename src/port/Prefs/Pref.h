#pragma once

// Setting types only. This header is included by every per-module settings header, so it
// deliberately pulls <nlohmann/json_fwd.hpp> rather than <nlohmann/json.hpp> — the JSON
// entry points below are declared against an incomplete type and defined in Setting_impl.h.
// Anything instantiating an uncommon Scalar<V> includes Setting_impl.h in its own .cpp and
// pays the parse cost alone; the common instantiations are listed at the bottom.

#include <stdint.h>

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "libultraship/color.h"
#include "Sections.h"

namespace Prefs {

const char* SectionBlock(PrefSection section);

struct ColorValue {
    Color_RGBA8 value{ 255, 255, 255, 255 };
    bool rainbow = false;
    bool locked = false;
};

// Color_RGBA8 has no operator==, and adding one at global scope would collide with the
// comparisons Menu.cpp defines in namespace Ship. Compare through this instead.
// Will be consolidated as this process moves forward.
template <typename V> inline bool ValueEquals(const V& a, const V& b) {
    return a == b;
}
template <> inline bool ValueEquals<Color_RGBA8>(const Color_RGBA8& a, const Color_RGBA8& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
template <> inline bool ValueEquals<ColorValue>(const ColorValue& a, const ColorValue& b) {
    return ValueEquals(a.value, b.value);
}

template <typename V>
concept HasSize = requires(const V& v) {
    v.size();
};

template <typename V> struct Options {
    const char* cvar = nullptr; // optional legacy name for console/remote lookup
    std::optional<V> min, max;  // arithmetic types only
    bool clamp = true;          // min/max and maxSize violations: clamp when true, reject when false
    size_t maxSize = 0;         // strings and containers; 0 = unlimited

    /// Allow-list: the value must equal one of these entries. Empty means unrestricted.
    std::vector<V> oneOf;

    std::function<bool(V&)> validator; ///< may adjust in place; false rejects

    Options& CVar(const char* cvar_) {
        cvar = cvar_;
        return *this;
    }
    Options& Min(V min_) {
        min = std::move(min_);
        return *this;
    }
    Options& Max(V max_) {
        max = std::move(max_);
        return *this;
    }
    Options& Clamp(bool clamp_) {
        clamp = clamp_;
        return *this;
    }
    Options& MaxSize(size_t maxSize_) {
        maxSize = maxSize_;
        return *this;
    }
    Options& OneOf(std::vector<V> oneOf_) {
        oneOf = std::move(oneOf_);
        return *this;
    }
    Options& Validator(std::function<bool(V&)> validator_) {
        validator = std::move(validator_);
        return *this;
    }
};

class Base {
public:
    Base(PrefSection section, std::string path, const char* cvar);
    virtual ~Base();

    Base(const Base&) = delete;
    Base& operator=(const Base&) = delete;

    PrefSection Section() const {
        return mSection;
    }
    // Dot-separated key within the section's root block.
    const std::string& Path() const {
        return mPath;
    }
    const std::string& FullPath() const {
        return mFullPath;
    }
    const char* CVar() const {
        return mCvar;
    }
    bool IsExplicitlySet() const {
        return mSet;
    }

    virtual bool IsDefault() const = 0;
    virtual void Reset() = 0;
    virtual void Write(nlohmann::json& out) const = 0;
    // Validates and applies. Returns false if the node was rejected, in which case the caller
    // leaves it in the document untouched rather than destroying it.
    virtual bool Read(const nlohmann::json& in) = 0;

    void SetOnChange(std::function<void(Base&)> callback) {
        mOnChange = std::move(callback);
    }

    void ApplyLoaded(const nlohmann::json& in) {
        if (Read(in)) {
            mSet = true;
        }
    }

protected:
    void MarkChanged(bool valueChanged);
    void MarkReset(bool valueChanged);

    PrefSection mSection;
    std::string mPath;
    std::string mFullPath;
    const char* mCvar;
    bool mSet = false;
    std::function<void(Base&)> mOnChange;
};

// One template covers every stored type. Containers work because nlohmann already serialises
// std::vector<T> as an array and std::map<std::string, V> as an object.
template <typename V> class Scalar : public Base {
public:
    Scalar(PrefSection section, std::string path, V def, Options<V> options = {})
        : Base(section, std::move(path), options.cvar), mValue(def), mDefault(std::move(def)),
          mOptions(std::move(options)) {
    }

    const V& Get() const {
        return mValue;
    }
    operator const V&() const {
        return mValue;
    }
    const V& Default() const {
        return mDefault;
    }

    void Set(V value) {
        if (!Validate(value)) {
            return;
        }
        const bool changed = !ValueEquals(mValue, value);
        mValue = std::move(value);
        mSet = true;
        MarkChanged(changed);
    }

    Scalar& operator=(V value) {
        Set(std::move(value));
        return *this;
    }

    bool IsDefault() const override {
        return ValueEquals(mValue, mDefault);
    }

    void Reset() override {
        const bool changed = !ValueEquals(mValue, mDefault);
        mValue = mDefault;
        mSet = false;
        MarkReset(changed);
    }

    void Write(nlohmann::json& out) const override;
    bool Read(const nlohmann::json& in) override;

    bool Validate(V& value) const;

    Options<V> GetOpts() {
        return mOptions;
    }

protected:
    V mValue;
    V mDefault;
    Options<V> mOptions;
};

using Bool = Scalar<bool>;
using Int32 = Scalar<int32_t>;
using Float = Scalar<float>;
using String = Scalar<std::string>;

class Color : public Scalar<ColorValue> {
public:
    Color(PrefSection section, std::string path, Color_RGBA8 def, Options<ColorValue> options = {})
        : Scalar<ColorValue>(section, std::move(path), ColorValue{ def }, std::move(options)) {
    }

    const Color_RGBA8& Value() const {
        return mValue.value;
    }
    bool Rainbow() const {
        return mValue.rainbow;
    }
    bool Locked() const {
        return mValue.locked;
    }
    const Color_RGBA8& DefaultValue() const {
        return mDefault.value;
    }

    void SetValue(Color_RGBA8 value);
    void SetRainbow(bool rainbow);
    void SetLocked(bool locked);
    void Randomize();
};

struct EnumEntry {
    const char* wireName;     // persisted in JSON; stable, never localised
    const char* displayLabel; // shown in the UI; safe to reword or localise
};

// An int32 in memory, an entry name on disk.
class Enum : public Scalar<int32_t> {
public:
    // Borrows a shared map. It must outlive the pref, so hand it over from a function-local
    // static rather than a namespace-scope global — a global in another translation unit may
    // still be zero-initialised when this constructor runs, and a zero-initialised std::map is
    // not a valid empty one.
    Enum(PrefSection section, std::string path, int32_t def, const std::map<int32_t, EnumEntry>& entries,
         Options<int32_t> options = {});

    // Takes ownership. Binds the map literal written at the declaration site, so a one-off enum
    // reads exactly as it did before and no temporary is left dangling.
    Enum(PrefSection section, std::string path, int32_t def, std::map<int32_t, EnumEntry>&& entries,
         Options<int32_t> options = {});

    // Never null: both constructors take a reference.
    const std::map<int32_t, EnumEntry>& Entries() const {
        return *mEntries;
    }

    const std::string& PendingWireName() const {
        return mPendingWireName;
    }

    void Write(nlohmann::json& out) const override;
    bool Read(const nlohmann::json& in) override;

protected:
    // Engaged only when this pref owns its map; mEntries points into it either way.
    std::optional<std::map<int32_t, EnumEntry>> mOwnedEntries;
    const std::map<int32_t, EnumEntry>* mEntries;
    std::string mPendingWireName;

private:
    void InstallEntryValidator();
};

// Stored as an int32 and divided by factor, because a float pref cannot represent 0.1 exactly and
// every save round-trips through the binary noise. factor is steps per whole unit, so 100 gives two
// decimal places. Options min/max are in stored units.
class Fixed : public Scalar<int32_t> {
public:
    Fixed(PrefSection section, std::string path, int32_t def, int32_t factor, Options<int32_t> options = {});

    int32_t Factor() const {
        return mFactor;
    }

    float Float() const {
        return (float)mValue / (float)mFactor;
    }

    float DefaultFloat() const {
        return (float)mDefault / (float)mFactor;
    }

    int32_t Decimals() const;

    // Rounds to the nearest stored unit.
    void SetFloat(float value);

protected:
    int32_t mFactor;
};

// Never instantiate Vector<bool>: std::vector<bool> is the proxy-reference specialisation and the
// const V& accessors above do not behave. Use Vector<uint8_t> for a flag array.
template <typename T> using Vector = Scalar<std::vector<T>>;
template <typename K, typename V> using Map = Scalar<std::map<K, V>>;

// Common instantiations live in Pref.cpp so this header never needs the full JSON parser.
extern template class Scalar<bool>;
extern template class Scalar<int32_t>;
extern template class Scalar<float>;
extern template class Scalar<std::string>;
extern template class Scalar<Color_RGBA8>;
extern template class Scalar<ColorValue>;
extern template class Scalar<std::vector<int32_t>>;
extern template class Scalar<std::vector<float>>;
extern template class Scalar<std::vector<std::string>>;
extern template class Scalar<std::map<std::string, int32_t>>;
extern template class Scalar<std::map<std::string, float>>;
extern template class Scalar<std::map<std::string, std::string>>;

} // namespace Prefs
