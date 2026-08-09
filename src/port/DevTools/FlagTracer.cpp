// Flag Tracer
//
// Logs every game-flag write with its enum name, optionally with the call stack
// that set it. A flag index alone is a bare number shared by unrelated actors.

#include <libultraship/bridge.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"

#include "FlagNames.generated.h"

#include <cstring>
#include <mutex>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#define LH_FLAGTRACE_STACKS_WIN 1
#elif defined(__linux__) || defined(__APPLE__)
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <cstdlib>
#define LH_FLAGTRACE_STACKS_POSIX 1
#endif

extern "C" {
#include "functions.h"
}

#define CVAR_FLAG_TRACE CVAR_DEVELOPER_TOOLS("FlagTrace")
#define CVAR_FLAG_TRACE_STACKS CVAR_DEVELOPER_TOOLS("FlagTraceStacks")
#define CVAR_FLAG_TRACE_SPACE CVAR_DEVELOPER_TOOLS("FlagTraceSpace")

namespace {

constexpr int32_t kSpaceAll = 0;

const char* SpaceName(int32_t flagSpace) {
    switch (flagSpace) {
        case ANCHOR_FLAGSPACE_FILE_PROGRESS:
            return "File Progress";
        case ANCHOR_FLAGSPACE_VOLATILE:
            return "Volatile";
        case ANCHOR_FLAGSPACE_LEVEL_SPECIFIC:
            return "Level";
        case ANCHOR_FLAGSPACE_MAP_SPECIFIC:
            return "Map";
        case ANCHOR_FLAGSPACE_RANDO_INF:
            return "Rando";
        default:
            return "Unknown";
    }
}

const char* LookupIn(const FlagName* table, int count, int index) {
    for (int i = 0; i < count; i++) {
        if (table[i].index == index) {
            return table[i].name;
        }
    }
    return nullptr;
}

template <int N> const char* LookupIn(const FlagName (&table)[N], int index) {
    return LookupIn(table, N, index);
}

// The enum name behind a flag index, or "" when the decomp hasn't named it.
std::string FlagLabel(int32_t flagSpace, int32_t index) {
    const char* name = nullptr;
    switch (flagSpace) {
        case ANCHOR_FLAGSPACE_FILE_PROGRESS:
            name = LookupIn(kFileProgressNames, index);
            break;
        case ANCHOR_FLAGSPACE_VOLATILE:
            name = LookupIn(kVolatileNames, index);
            break;
        case ANCHOR_FLAGSPACE_LEVEL_SPECIFIC:
            name = LookupIn(kLevelNames, index);
            break;
        case ANCHOR_FLAGSPACE_MAP_SPECIFIC: {
            const int level = (int)map_getLevel(gsworld_getMap());
            std::string joined;
            for (const auto& group : kMapFlagGroups) {
                if (group.level != level) {
                    continue;
                }
                if (const char* candidate = LookupIn(group.names, group.count, index)) {
                    if (!joined.empty()) {
                        joined += " | ";
                    }
                    joined += candidate;
                }
            }
            return joined;
        }
        default:
            break;
    }
    return name != nullptr ? name : "";
}

// Symbolized stack of whatever led here.
std::string CaptureCallerStack(int maxFrames) {
    auto isPlumbing = [](const char* name) {
        static const char* kPlumbing[] = { "FlagTrace",    "EventSystem", "CallEvent", "Flag_set",   "Flags_set",
                                           "Flag_setEx",   "Flags_setEx", "Flag_setN", "Flags_setN", "Flag_getAndSet",
                                           "Flags_getSet", "invoke",      "std::",     "lambda" };
        for (const char* p : kPlumbing) {
            if (std::strstr(name, p) != nullptr) {
                return true;
            }
        }
        return false;
    };

#if defined(LH_FLAGTRACE_STACKS_WIN)
    static std::once_flag symOnce;
    static std::mutex symMutex;
    HANDLE proc = GetCurrentProcess();
    std::call_once(symOnce, [proc] {
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        SymInitialize(proc, nullptr, TRUE);
    });

    void* frames[48] = {};
    const USHORT captured = CaptureStackBackTrace(1, 48, frames, nullptr);

    // dbghelp is not thread safe and its handles are process-wide.
    std::lock_guard<std::mutex> lock(symMutex);

    alignas(SYMBOL_INFO) char symBuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
    auto* sym = reinterpret_cast<SYMBOL_INFO*>(symBuf);
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym->MaxNameLen = MAX_SYM_NAME;

    std::string out;
    int shown = 0;
    for (USHORT i = 0; i < captured && shown < maxFrames; i++) {
        DWORD64 disp = 0;
        if (!SymFromAddr(proc, (DWORD64)frames[i], &disp, sym)) {
            continue;
        }
        if (out.empty() && isPlumbing(sym->Name)) {
            continue;
        }
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(line);
        DWORD lineDisp = 0;
        if (SymGetLineFromAddr64(proc, (DWORD64)frames[i], &lineDisp, &line)) {
            const char* file = std::strrchr(line.FileName, '\\');
            file = file ? file + 1 : line.FileName;
            out += fmt::format("\n      {} ({}:{})", sym->Name, file, line.LineNumber);
        } else {
            out += fmt::format("\n      {}", sym->Name);
        }
        shown++;
    }
    return out;

#elif defined(LH_FLAGTRACE_STACKS_POSIX)
    void* frames[48] = {};
    const int captured = backtrace(frames, 48);

    std::string out;
    int shown = 0;
    for (int i = 1; i < captured && shown < maxFrames; i++) {
        Dl_info info = {};
        if (dladdr(frames[i], &info) == 0 || info.dli_sname == nullptr) {
            out += fmt::format("\n      {}", frames[i]);
            shown++;
            continue;
        }

        int status = 0;
        char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        const char* name = (status == 0 && demangled != nullptr) ? demangled : info.dli_sname;
        const bool skip = out.empty() && isPlumbing(name);
        if (!skip) {
            out += fmt::format("\n      {}", name);
            shown++;
        }
        std::free(demangled);
    }
    return out;

#else
    (void)maxFrames;
    (void)isPlumbing;
    return "\n      (call stacks are unavailable on this platform)";
#endif
}

} // namespace

void RegisterFlagTracer_Init() {
    const bool enabled = CVarGetInteger(CVAR_FLAG_TRACE, 0) != 0;

    COND_HOOK(OnGameFlagSet, EVENT_PRIORITY_NORMAL, enabled, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnGameFlagSet*>(event);

        const int32_t spaceFilter = CVarGetInteger(CVAR_FLAG_TRACE_SPACE, kSpaceAll);
        if (spaceFilter != kSpaceAll && ev->flagSpace != (spaceFilter - 1)) {
            return;
        }

        std::string where;
        if (ev->length > 1) {
            where = fmt::format("0x{:X}-0x{:X}", ev->index, ev->index + ev->length - 1);
        } else {
            const std::string label = FlagLabel(ev->flagSpace, ev->index);
            where = label.empty() ? fmt::format("0x{:X}", ev->index) : fmt::format("0x{:X} {}", ev->index, label);
        }

        std::string stack;
        if (CVarGetInteger(CVAR_FLAG_TRACE_STACKS, 0)) {
            stack = fmt::format("\n    set by:{}", CaptureCallerStack(6));
        }

        SPDLOG_INFO("[Flag] {:<13} {} -> {}{}", SpaceName(ev->flagSpace), where, ev->value ? "ON" : "off", stack);
    });

    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, enabled, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnMapLoad*>(event);
        SPDLOG_INFO("[Flag] ===== map 0x{:X} -> 0x{:X} (level 0x{:X}, exit {}) =====", (int)ev->prevMap,
                    (int)ev->nextMap, (int)map_getLevel((enum map_e)ev->nextMap), ev->exit);
    });
}

static RegisterShipInitFunc initFlagTracer(RegisterFlagTracer_Init, { CVAR_FLAG_TRACE });
