#include "port/Resource/Alt/AltPathPool.h"

#include <deque>
#include <mutex>
#include <unordered_map>

const char* InternAltPath(const std::string& path) {
    static std::mutex sMutex;
    static std::unordered_map<std::string, const char*> sCache;
    static std::deque<std::string> sPool;

    std::lock_guard<std::mutex> lock(sMutex);
    auto it = sCache.find(path);
    if (it != sCache.end()) {
        return it->second;
    }
    sPool.push_back(path);
    const char* p = sPool.back().c_str();
    sCache.emplace(path, p);
    return p;
}
