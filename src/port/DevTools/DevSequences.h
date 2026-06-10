#pragma once

namespace Lighthouse {
namespace DevTools {

enum DevSequenceId {
    SEQ_NONE = 0,
    SEQ_PARADE_FF,
    SEQ_PARADE_FINAL,
    SEQ_MODE9_BK,
    SEQ_MODE9_DIALOG,
    SEQ_ATTRACT_BASE,
};

constexpr int ATTRACT_DEMO_COUNT = 10;
void RequestSequence(int seq);

} // namespace DevTools
} // namespace Lighthouse
