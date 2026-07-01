#include "AssetTrace.h"

#include <mutex>
#include <string>
#include <unordered_set>

#include <spdlog/spdlog.h>

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
#elif defined(__has_include)
#if __has_include(<execinfo.h>)
#define LH_HAVE_EXECINFO 1
#include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <fstream>
#endif
#endif

#if defined(LH_HAVE_EXECINFO)
// True when a debugger/tracer is attached, so we can SIGTRAP without crashing an
// ordinary end-user run. Only Linux exposes this cheaply; elsewhere assume none.
static bool PortDebuggerAttached() {
#if defined(__linux__)
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.rfind("TracerPid:", 0) == 0) {
            return std::strtol(line.c_str() + 10, nullptr, 10) != 0;
        }
    }
#endif
    return false;
}
#endif

void port_traceBadAssetId(uint32_t assetId) {
    static std::mutex seenMutex;
    static std::unordered_set<uint32_t> seen;
    {
        std::lock_guard<std::mutex> lock(seenMutex);
        if (!seen.insert(assetId).second) {
            return;
        }
    }

#if defined(_WIN32)
    void* frames[32];
    USHORT n = CaptureStackBackTrace(1, 32, frames, nullptr);

    HANDLE proc = GetCurrentProcess();
    static std::once_flag symOnce;
    std::call_once(symOnce, [proc] {
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        SymInitialize(proc, nullptr, TRUE);
    });

    SPDLOG_WARN("[AssetTrace] invalid asset id {} backtrace:", assetId);

    alignas(SYMBOL_INFO) char symBuf[sizeof(SYMBOL_INFO) + 256] = {};
    auto* sym = reinterpret_cast<SYMBOL_INFO*>(symBuf);
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym->MaxNameLen = 255;

    for (USHORT i = 0; i < n; ++i) {
        auto addr = reinterpret_cast<DWORD64>(frames[i]);
        DWORD64 symDisp = 0;
        DWORD lineDisp = 0;
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(line);

        if (SymFromAddr(proc, addr, &symDisp, sym)) {
            if (SymGetLineFromAddr64(proc, addr, &lineDisp, &line)) {
                SPDLOG_WARN("    #{:02} {}+0x{:x}  ({}:{})", i, sym->Name, symDisp, line.FileName,
                            line.LineNumber);
            } else {
                SPDLOG_WARN("    #{:02} {}+0x{:x}", i, sym->Name, symDisp);
            }
        } else {
            SPDLOG_WARN("    #{:02} 0x{:x}", i, addr);
        }
    }

    if (IsDebuggerPresent()) {
        __debugbreak();
    }
#elif defined(LH_HAVE_EXECINFO)
    void* frames[32];
    int n = backtrace(frames, 32);
    char** symbols = backtrace_symbols(frames, n);

    SPDLOG_WARN("[AssetTrace] invalid asset id {} backtrace:", assetId);

    for (int i = 1; i < n; ++i) {
        SPDLOG_WARN("    #{:02} {}", i, symbols ? symbols[i] : "?");
    }
    free(symbols);

    if (PortDebuggerAttached()) {
        raise(SIGTRAP);
    }
#else
    SPDLOG_WARN("[AssetTrace] invalid asset id {} (backtrace unavailable on this platform)",
                assetId);
#endif
}
