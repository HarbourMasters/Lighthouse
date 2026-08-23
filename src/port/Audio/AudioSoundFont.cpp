#include <string.h>
#include <stdlib.h>
#include <stdint.h>

extern "C" {
#include <ultra64.h>
#include <PR/libaudio.h>
}

#include <libultraship/libultraship.h>
#include <libultraship/libultra/AudioDmaRegistry.h>
#include <ship/resource/type/Blob.h>
#include <spdlog/spdlog.h>

#include <map>
#include <string>
#include <vector>

namespace {

class BodyReader {
public:
    BodyReader(const uint8_t* data, size_t size) : mData(data), mSize(size) {
    }

    bool Ok() const {
        return !mFailed;
    }

    uint8_t Read8() {
        if (mPos + 1 > mSize) {
            mFailed = true;
            return 0;
        }
        return mData[mPos++];
    }
    int8_t ReadS8() {
        return static_cast<int8_t>(Read8());
    }
    int16_t ReadS16() {
        const uint16_t lo = Read8();
        const uint16_t hi = Read8();
        return static_cast<int16_t>(lo | (hi << 8));
    }
    uint32_t Read32() {
        const uint32_t a = Read8(), b = Read8(), c = Read8(), d = Read8();
        return a | (b << 8) | (c << 16) | (d << 24);
    }
    int32_t ReadS32() {
        return static_cast<int32_t>(Read32());
    }

    std::string Str() {
        const uint32_t len = Read32();
        if (mFailed || mPos + len > mSize) {
            mFailed = true;
            return std::string();
        }
        std::string s(reinterpret_cast<const char*>(mData + mPos), len);
        mPos += len;
        return s;
    }

    std::vector<uint8_t> Bytes(size_t n) {
        if (mFailed || mPos + n > mSize) {
            mFailed = true;
            return {};
        }
        std::vector<uint8_t> v(mData + mPos, mData + mPos + n);
        mPos += n;
        return v;
    }

private:
    const uint8_t* mData;
    size_t mSize;
    size_t mPos = 0;
    bool mFailed = false;
};

struct ParsedSound {
    uint8_t samplePan = 0, sampleVolume = 0, flags = 0;
    bool hasEnvelope = false;
    ALEnvelope envelope{};
    bool hasKeyMap = false;
    ALKeyMap keyMap{};
    bool hasWave = false;
    uint8_t waveType = 0, waveFlags = 0;
    bool hasLoop = false;
    uint32_t loopStart = 0, loopEnd = 0, loopCount = 0;
    std::vector<int16_t> loopState;
    bool hasBook = false;
    int32_t bookOrder = 0, bookNpredictors = 0;
    std::vector<int16_t> book;
    std::vector<uint8_t> data;
};

struct DescInstrument {
    uint8_t volume = 0, pan = 0, priority = 0, flags = 0;
    uint8_t tremType = 0, tremRate = 0, tremDepth = 0, tremDelay = 0;
    uint8_t vibType = 0, vibRate = 0, vibDepth = 0, vibDelay = 0;
    int16_t bendRange = 0;
    std::vector<std::string> sounds;
};

struct DescBank {
    int32_t sampleRate = 0;
    uint8_t flags = 0, pad = 0;
    std::vector<bool> present;
    std::vector<DescInstrument> instruments;
};

std::vector<void*> sBankAllocations;
std::vector<std::shared_ptr<Ship::IResource>> sBankResources;

void* Keep(void* p) {
    sBankAllocations.push_back(p);
    return p;
}

const Ship::Blob* LoadBody(const std::string& path) {
    auto rm = Ship::Context::GetRawInstance()->GetResourceManager();
    auto res = rm->LoadResource(path);
    if (res == nullptr) {
        return nullptr;
    }
    auto blob = std::dynamic_pointer_cast<Ship::Blob>(res);
    if (blob == nullptr) {
        return nullptr;
    }
    sBankResources.push_back(res);
    return blob.get();
}

bool ReadInstrument(BodyReader& r, DescInstrument& inst) {
    inst.volume = r.Read8();
    inst.pan = r.Read8();
    inst.priority = r.Read8();
    inst.flags = r.Read8();
    inst.tremType = r.Read8();
    inst.tremRate = r.Read8();
    inst.tremDepth = r.Read8();
    inst.tremDelay = r.Read8();
    inst.vibType = r.Read8();
    inst.vibRate = r.Read8();
    inst.vibDepth = r.Read8();
    inst.vibDelay = r.Read8();
    inst.bendRange = r.ReadS16();
    const uint32_t count = r.Read32();
    if (!r.Ok() || count > 0x1000) {
        return false;
    }
    inst.sounds.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        inst.sounds.push_back(r.Str());
    }
    return r.Ok();
}

bool ParseSoundBody(const uint8_t* data, size_t size, ParsedSound& out) {
    BodyReader r(data, size);
    out.samplePan = r.Read8();
    out.sampleVolume = r.Read8();
    out.flags = r.Read8();

    out.hasEnvelope = r.Read8() != 0;
    if (out.hasEnvelope) {
        out.envelope.attackTime = r.ReadS32();
        out.envelope.decayTime = r.ReadS32();
        out.envelope.releaseTime = r.ReadS32();
        out.envelope.attackVolume = r.Read8();
        out.envelope.decayVolume = r.Read8();
    }

    out.hasKeyMap = r.Read8() != 0;
    if (out.hasKeyMap) {
        out.keyMap.velocityMin = r.Read8();
        out.keyMap.velocityMax = r.Read8();
        out.keyMap.keyMin = r.Read8();
        out.keyMap.keyMax = r.Read8();
        out.keyMap.keyBase = r.Read8();
        out.keyMap.detune = r.ReadS8();
    }

    out.hasWave = r.Read8() != 0;
    if (out.hasWave) {
        out.waveType = r.Read8();
        out.waveFlags = r.Read8();

        out.hasLoop = r.Read8() != 0;
        if (out.hasLoop) {
            out.loopStart = r.Read32();
            out.loopEnd = r.Read32();
            out.loopCount = r.Read32();
            const uint32_t n = r.Read32();
            if (!r.Ok() || n > 64) {
                return false;
            }
            out.loopState.reserve(n);
            for (uint32_t i = 0; i < n; i++) {
                out.loopState.push_back(r.ReadS16());
            }
        }

        out.hasBook = r.Read8() != 0;
        if (out.hasBook) {
            out.bookOrder = r.ReadS32();
            out.bookNpredictors = r.ReadS32();
            const uint32_t n = r.Read32();
            if (!r.Ok() || n > 0x4000) {
                return false;
            }
            out.book.reserve(n);
            for (uint32_t i = 0; i < n; i++) {
                out.book.push_back(r.ReadS16());
            }
        }

        const uint32_t dataSize = r.Read32();
        out.data = r.Bytes(dataSize);
    }
    return r.Ok();
}

ALSound* BuildSound(const ParsedSound& parsed, uint8_t* sampleBase) {
    auto* sound = static_cast<ALSound*>(Keep(malloc(sizeof(ALSound))));
    memset(sound, 0, sizeof(ALSound));
    sound->samplePan = parsed.samplePan;
    sound->sampleVolume = parsed.sampleVolume;
    sound->flags = parsed.flags;

    if (parsed.hasEnvelope) {
        auto* env = static_cast<ALEnvelope*>(Keep(malloc(sizeof(ALEnvelope))));
        *env = parsed.envelope;
        sound->envelope = env;
    }
    if (parsed.hasKeyMap) {
        auto* km = static_cast<ALKeyMap*>(Keep(malloc(sizeof(ALKeyMap))));
        *km = parsed.keyMap;
        sound->keyMap = km;
    }
    if (!parsed.hasWave) {
        return sound;
    }

    auto* wt = static_cast<ALWaveTable*>(Keep(malloc(sizeof(ALWaveTable))));
    memset(wt, 0, sizeof(ALWaveTable));
    wt->base = sampleBase;
    wt->len = static_cast<s32>(parsed.data.size());
    wt->type = parsed.waveType;
    wt->flags = parsed.waveFlags;

    if (parsed.hasLoop) {
        auto* loop = static_cast<ALADPCMloop*>(Keep(malloc(sizeof(ALADPCMloop))));
        memset(loop, 0, sizeof(ALADPCMloop));
        loop->start = parsed.loopStart;
        loop->end = parsed.loopEnd;
        loop->count = parsed.loopCount;
        for (size_t i = 0; i < parsed.loopState.size() && i < 16; i++) {
            loop->state[i] = parsed.loopState[i];
        }
        wt->waveInfo.adpcmWave.loop = loop;
    }
    if (parsed.hasBook) {
        const size_t entries = parsed.book.size();
        const size_t allocSize = sizeof(ALADPCMBook) + (entries > 1 ? (entries - 1) * sizeof(s16) : 0);
        auto* book = static_cast<ALADPCMBook*>(Keep(malloc(allocSize)));
        memset(book, 0, allocSize);
        book->order = parsed.bookOrder;
        book->npredictors = parsed.bookNpredictors;
        for (size_t i = 0; i < entries; i++) {
            book->book[i] = parsed.book[i];
        }
        wt->waveInfo.adpcmWave.book = book;
    }

    sound->wavetable = wt;
    return sound;
}

ALInstrument* BuildInstrument(const DescInstrument& desc, const std::map<std::string, ALSound*>& sounds) {
    const size_t count = desc.sounds.size();
    const size_t allocSize = sizeof(ALInstrument) + (count > 1 ? (count - 1) * sizeof(ALSound*) : 0);
    auto* inst = static_cast<ALInstrument*>(Keep(malloc(allocSize)));
    memset(inst, 0, allocSize);

    inst->volume = desc.volume;
    inst->pan = desc.pan;
    inst->priority = desc.priority;
    inst->flags = desc.flags;
    inst->tremType = desc.tremType;
    inst->tremRate = desc.tremRate;
    inst->tremDepth = desc.tremDepth;
    inst->tremDelay = desc.tremDelay;
    inst->vibType = desc.vibType;
    inst->vibRate = desc.vibRate;
    inst->vibDepth = desc.vibDepth;
    inst->vibDelay = desc.vibDelay;
    inst->bendRange = desc.bendRange;
    inst->soundCount = static_cast<s16>(count);

    for (size_t i = 0; i < count; i++) {
        const auto it = desc.sounds[i].empty() ? sounds.end() : sounds.find(desc.sounds[i]);
        inst->soundArray[i] = (it != sounds.end()) ? it->second : nullptr;
    }
    return inst;
}

} // namespace

extern "C" ALBankFile* port_alBnkfLoad(const char* descriptorPath) {
    const Ship::Blob* descriptor = LoadBody(descriptorPath);
    if (descriptor == nullptr) {
        SPDLOG_ERROR("[Audio] soundfont descriptor '{}' is missing", descriptorPath);
        return nullptr;
    }

    BodyReader r(descriptor->Data.data(), descriptor->Data.size());
    const uint32_t bankCount = r.Read32();
    if (!r.Ok() || bankCount == 0 || bankCount > 16) {
        SPDLOG_ERROR("[Audio] soundfont descriptor '{}' declares {} banks", descriptorPath, bankCount);
        return nullptr;
    }

    std::vector<DescBank> banks(bankCount);
    for (uint32_t b = 0; b < bankCount; b++) {
        DescBank& bank = banks[b];
        bank.sampleRate = r.ReadS32();
        bank.flags = r.Read8();
        bank.pad = r.Read8();
        const uint32_t instCount = r.Read32();
        if (!r.Ok() || instCount > 0x1000) {
            SPDLOG_ERROR("[Audio] '{}' bank {}: {} instruments", descriptorPath, b, instCount);
            return nullptr;
        }
        bank.present.resize(instCount);
        bank.instruments.resize(instCount);
        for (uint32_t i = 0; i < instCount; i++) {
            bank.present[i] = r.Read8() != 0;
            if (bank.present[i] && !ReadInstrument(r, bank.instruments[i])) {
                SPDLOG_ERROR("[Audio] '{}' bank {}: instrument {} is malformed", descriptorPath, b, i);
                return nullptr;
            }
        }
    }
    if (!r.Ok()) {
        SPDLOG_ERROR("[Audio] soundfont descriptor '{}' ended early", descriptorPath);
        return nullptr;
    }

    std::vector<std::string> order;
    std::map<std::string, ParsedSound> parsed;
    auto want = [&](const DescInstrument& inst) {
        for (const std::string& path : inst.sounds) {
            if (path.empty() || parsed.count(path) != 0) {
                continue;
            }
            const Ship::Blob* body = LoadBody(path);
            ParsedSound sound;
            if (body == nullptr || !ParseSoundBody(body->Data.data(), body->Data.size(), sound)) {
                SPDLOG_ERROR("[Audio] sound resource '{}' is missing or malformed", path);
                continue;
            }
            parsed.emplace(path, std::move(sound));
            order.push_back(path);
        }
    };
    for (const DescBank& bank : banks) {
        for (size_t i = 0; i < bank.instruments.size(); i++) {
            if (bank.present[i]) {
                want(bank.instruments[i]);
            }
        }
    }

    size_t arenaSize = 0;
    for (const std::string& path : order) {
        arenaSize = (arenaSize + parsed[path].data.size() + 0xF) & ~static_cast<size_t>(0xF);
    }
    auto* arena = static_cast<uint8_t*>(Keep(calloc(arenaSize ? arenaSize : 1, 1)));
    AudioDma_Register(arena, arenaSize);

    std::map<std::string, ALSound*> sounds;
    size_t cursor = 0;
    for (const std::string& path : order) {
        const ParsedSound& p = parsed[path];
        uint8_t* base = arena + cursor;
        if (!p.data.empty()) {
            memcpy(base, p.data.data(), p.data.size());
        }
        cursor = (cursor + p.data.size() + 0xF) & ~static_cast<size_t>(0xF);
        sounds.emplace(path, BuildSound(p, base));
    }

    const size_t fileSize = sizeof(ALBankFile) + (bankCount > 1 ? (bankCount - 1) * sizeof(ALBank*) : 0);
    auto* file = static_cast<ALBankFile*>(Keep(malloc(fileSize)));
    memset(file, 0, fileSize);
    file->revision = AL_BANK_VERSION;
    file->bankCount = static_cast<s16>(bankCount);

    for (uint32_t b = 0; b < bankCount; b++) {
        const DescBank& desc = banks[b];
        const size_t instCount = desc.instruments.size();
        const size_t bankSize = sizeof(ALBank) + (instCount > 1 ? (instCount - 1) * sizeof(ALInstrument*) : 0);
        auto* bank = static_cast<ALBank*>(Keep(malloc(bankSize)));
        memset(bank, 0, bankSize);

        bank->instCount = static_cast<s16>(instCount);
        bank->flags = desc.flags;
        bank->pad = desc.pad;
        bank->sampleRate = desc.sampleRate;
        for (size_t i = 0; i < instCount; i++) {
            bank->instArray[i] = desc.present[i] ? BuildInstrument(desc.instruments[i], sounds) : nullptr;
        }
        file->bankArray[b] = bank;
    }

    SPDLOG_INFO("[Audio] '{}': {} bank(s), {} sounds, {} KB of samples", descriptorPath, bankCount, order.size(),
                arenaSize / 1024);
    return file;
}

extern "C" void port_alBnkfReleaseSources(void) {
    sBankResources.clear();
}

extern "C" void port_alBnkfFreeAll(void) {
    for (void* p : sBankAllocations) {
        free(p);
    }
    sBankAllocations.clear();
    sBankResources.clear();
}
