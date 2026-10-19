#pragma once

#include <memory>
#include <string>

namespace Lighthouse {

class FrameHost {
public:
    virtual ~FrameHost() = default;
    virtual void Ready() = 0;
    virtual bool WantsFrame() const = 0;
    virtual void Frame() = 0;
    virtual void Poll() {
    }
    virtual bool HoldQuit() {
        return false;
    }
};

struct RegisterFrameHost {
    RegisterFrameHost(const char* flag, std::unique_ptr<FrameHost> (*make)());
};

bool IsFrameHostFlag(const std::string& arg);
FrameHost* GetFrameHost();
void WakeGame();

} // namespace Lighthouse
