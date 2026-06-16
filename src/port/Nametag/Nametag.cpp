#include "Nametag.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <libultraship/libultraship.h>
#include <libultraship/bridge.h>
#include <imgui.h>
#include <ship/window/gui/GuiWindow.h>
#include <fast/Fast3dWindow.h>
#include "port/UI/cvar_prefixes.h"
#include "port/ShipInit.hpp"

namespace {

// Tag appearance
constexpr ImU32 kBgColor = IM_COL32(0, 0, 0, 200);
constexpr ImU32 kBorderColor = IM_COL32(64, 128, 255, 255);
constexpr ImU32 kTextColor = IM_COL32(255, 255, 255, 255);
constexpr float kPaddingX = 6.0f;
constexpr float kPaddingY = 3.0f;
constexpr float kCornerRadius = 3.0f;
constexpr float kBorderThickness = 1.5f;

// Aspect ratios. kGameAspect is what AdjXForAspectRatio preserves in-FB.
// kLowResAspect is the aspect Gui::DrawGame enforces under CVAR_LOW_RES_MODE==1.
constexpr float kGameAspect = 4.0f / 3.0f;
constexpr float kLowResAspect = 320.0f / 240.0f;

struct Entry {
    float pos[3];
    std::string text;
};

std::vector<Entry> sQueue;
Nametag::ProjectFn sProject = nullptr;
const int* sFbWidth = nullptr;
const int* sFbHeight = nullptr;

// Mirrors LUS Gui::GetIntegerScaleFactor; mGameWindowViewport lags one
// frame, matching DrawGame's own read order.
int16_t ComputeIntegerScaleFactor(const std::shared_ptr<Fast::Interpreter>& interp) {
    const auto& gvp = interp->mGameWindowViewport;
    const auto& cur = interp->mCurDimensions;
    const uint32_t curW = std::max<uint32_t>(1, cur.width);
    const uint32_t curH = std::max<uint32_t>(1, cur.height);

    const bool fitAuto = CVarGetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".IntegerScale.FitAutomatically", 0) != 0;
    if (!fitAuto) {
        int16_t factor =
            static_cast<int16_t>(CVarGetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".IntegerScale.Factor", 1));
        const bool neverExceed =
            CVarGetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".IntegerScale.NeverExceedBounds", 1) != 0;
        if (neverExceed && gvp.width > 0 && gvp.height > 0) {
            if (((float)gvp.height / (float)gvp.width) < ((float)curH / (float)curW)) {
                if ((uint32_t)factor > gvp.height / curH) {
                    factor = static_cast<int16_t>(gvp.height / curH);
                }
            } else {
                if ((uint32_t)factor > gvp.width / curW) {
                    factor = static_cast<int16_t>(gvp.width / curW);
                }
            }
        }
        return factor < 1 ? 1 : factor;
    }

    int16_t factor = 1;
    if (gvp.width > 0 && gvp.height > 0) {
        if (((float)gvp.height / (float)gvp.width) < ((float)curH / (float)curW)) {
            factor = static_cast<int16_t>(gvp.height / curH);
        } else {
            factor = static_cast<int16_t>(gvp.width / curW);
        }
    }
    factor += static_cast<int16_t>(CVarGetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".IntegerScale.ExceedBoundsBy", 0));
    return factor < 1 ? 1 : factor;
}

class NametagOverlay : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;
    void InitElement() override {
    }
    void UpdateElement() override {
    }
    void DrawElement() override {
    }
    void Draw() override {
        // Suppress the overlay while the menu is up so tags don't cover menu items.
        auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();
        if (gui && gui->GetMenuOrMenubarVisible()) {
            return;
        }

        if (sProject == nullptr || sFbWidth == nullptr || sFbHeight == nullptr) {
            return;
        }

        // The projection emits coords in the game's native framebuffer space.
        // AdjXForAspectRatio preserves a 4:3 image inside the framebuffer (no Y
        // adjustment), so we anchor in the on-screen 4:3 content rect, composed
        // as: Main Game window -> game image rect (resolution-mode dependent) ->
        // 4:3 inset. IgnoreAspectCorrection stretches the FB non-uniformly and is
        // handled as a separate branch.
        auto fastWnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());
        auto interp = fastWnd ? fastWnd->GetInterpreterWeak().lock() : nullptr;
        if (interp == nullptr) {
            return;
        }

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const float mgOriginX = vp->WorkPos.x;
        const float mgOriginY = vp->WorkPos.y;
        const float mgW = vp->WorkSize.x;
        const float mgH = vp->WorkSize.y;

        const int lowResMode = CVarGetInteger(CVAR_LOW_RES_MODE, 0);
        const bool advResEnabled = CVarGetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".Enabled", 0) != 0;
        const bool pixelPerfect =
            advResEnabled && CVarGetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".PixelPerfectMode", 0) != 0;
        const bool ignoreAspect = advResEnabled && !pixelPerfect &&
                                  CVarGetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".IgnoreAspectCorrection", 0) != 0;

        const float fbW = std::max(1.0f, static_cast<float>(interp->mCurDimensions.width));
        const float fbH = std::max(1.0f, static_cast<float>(interp->mCurDimensions.height));
        const float fbAspect = (lowResMode == 1) ? kLowResAspect : (fbW / fbH);

        // Game image rect � where Gui::DrawGame blits the framebuffer.
        float gameRectW, gameRectH;
        if (lowResMode == 1) {
            gameRectH = mgH;
            gameRectW = mgH * kGameAspect;
            if (gameRectW > mgW) {
                gameRectW = mgW;
                gameRectH = mgW / kGameAspect;
            }
        } else if (pixelPerfect) {
            const int16_t factor = ComputeIntegerScaleFactor(interp);
            gameRectW = fbW * factor;
            gameRectH = fbH * factor;
        } else if (ignoreAspect) {
            gameRectW = mgW;
            gameRectH = mgH;
        } else {
            if (mgH * fbAspect <= mgW) {
                gameRectH = mgH;
                gameRectW = mgH * fbAspect;
            } else {
                gameRectW = mgW;
                gameRectH = mgW / fbAspect;
            }
        }
        const float gameRectX = mgOriginX + (mgW - gameRectW) * 0.5f;
        const float gameRectY = mgOriginY + (mgH - gameRectH) * 0.5f;

        // 4:3 content rect inside the game image rect. AdjXForAspectRatio makes the
        // 4:3 content span the full FB height with width = fbH*(4/3). Aspect-correct
        // modes scale it uniformly into gameRect; ignoreAspect stretches X by
        // gameRectW/fbW independently of Y.
        float contentW, contentH;
        if (ignoreAspect) {
            contentW = fbH * kGameAspect * (gameRectW / fbW);
            contentH = gameRectH;
        } else {
            contentW = gameRectH * kGameAspect;
            contentH = gameRectH;
        }
        const float originX = gameRectX + (gameRectW - contentW) * 0.5f;
        const float originY = gameRectY + (gameRectH - contentH) * 0.5f;
        const float scaleX = contentW / static_cast<float>(*sFbWidth);
        const float scaleY = contentH / static_cast<float>(*sFbHeight);

        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        for (const auto& entry : sQueue) {
            float pos[3] = { entry.pos[0], entry.pos[1], entry.pos[2] };
            float screen[2];
            if (sProject(pos, screen)) {
                ImVec2 textSize = ImGui::CalcTextSize(entry.text.c_str());
                ImVec2 anchor(originX + screen[0] * scaleX, originY + screen[1] * scaleY);
                ImVec2 textTL(anchor.x - textSize.x * 0.5f, anchor.y - textSize.y * 0.5f);
                ImVec2 boxTL(textTL.x - kPaddingX, textTL.y - kPaddingY);
                ImVec2 boxBR(textTL.x + textSize.x + kPaddingX, textTL.y + textSize.y + kPaddingY);
                drawList->AddRectFilled(boxTL, boxBR, kBgColor, kCornerRadius);
                drawList->AddRect(boxTL, boxBR, kBorderColor, kCornerRadius, 0, kBorderThickness);
                drawList->AddText(textTL, kTextColor, entry.text.c_str());
            }
        }
    }
};

} // namespace

namespace Nametag {

void SetProjectFn(ProjectFn fn) {
    sProject = fn;
}

void SetNativeFramebufferSize(const int* width, const int* height) {
    sFbWidth = width;
    sFbHeight = height;
}

void RegisterOverlay() {
    auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();
    auto overlay = std::make_shared<NametagOverlay>(CVAR_WINDOW("NametagOverlay"), "Nametag Overlay");
    gui->AddGuiWindow(overlay);
    overlay->Show();
}

void Clear() {
    sQueue.clear();
}

void Push(float x, float y, float z, const char* label) {
    if (label == nullptr) {
        return;
    }
    Entry entry;
    entry.pos[0] = x;
    entry.pos[1] = y;
    entry.pos[2] = z;
    entry.text = label;
    sQueue.push_back(std::move(entry));
}

} // namespace Nametag