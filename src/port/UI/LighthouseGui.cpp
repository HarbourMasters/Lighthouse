#include "LighthouseGui.hpp"

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_internal.h>
#include "UIWidgets.hpp"
#include "src/port/DevTools/EventDebugger.h"
#include "src/port/DevTools/OcclusionDebug.h"

#ifdef __APPLE__
#include <fast/backends/gfx_metal.h>
#endif

#ifdef __SWITCH__
#include <port/switch/SwitchImpl.h>
#endif

// #include "Enhancements/Trackers/ItemTracker/ItemTracker.h"
// #include "Enhancements/Trackers/ItemTracker/ItemTrackerSettings.h"
#include "port/Enhancements/Trackers/DisplayOverlay.h"
// #include "Enhancements/Trackers//TimeSplits/Timesplits.h"
// #include "Enhancements/Trackers/TimeSplits/TimesplitsSettings.h"
#include "port/Rando/CheckTracker/CheckTracker.h"

#include "Notification.h"
#include "port/Controller/Mapper.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Backports/EggAim.h"
#include "LighthouseMenu.h"
#include "LighthouseInputEditorWindow.h"
#include "LighthouseModMenuWindow.h"
// #include "DeveloperTools/HookDebugger.h"
#include "DeveloperTools/SaveEditor.h"
#include "DeveloperTools/GameplayTools.h"
// #include "DeveloperTools/ActorViewer.h"
// #include "DeveloperTools/CollisionViewer.h"
// #include "DeveloperTools/EventLog.h"
// #include "DeveloperTools/DLViewer.h"
// #include "DeveloperTools/MessageViewer.h"

namespace LighthouseGui {
// MARK: - Delegates

std::shared_ptr<Ship::GuiWindow> mConsoleWindow;
std::shared_ptr<Ship::GuiWindow> mStatsWindow;
std::shared_ptr<Ship::GuiWindow> mGfxDebuggerWindow;
std::shared_ptr<LighthouseInputEditorWindow> mInputEditorWindow;
std::shared_ptr<Mapper::MapperWindow> mGamepadMapperWindow;
std::shared_ptr<LighthouseModMenuWindow> mModMenuWindow;
std::shared_ptr<LighthouseRomhackMenuWindow> mRomhackMenuWindow;

// std::shared_ptr<HookDebuggerWindow> mHookDebuggerWindow;
std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
std::shared_ptr<GameplayToolsWindow> mGameplayToolsWindow;
// std::shared_ptr<HudEditorWindow> mHudEditorWindow;
// std::shared_ptr<CosmeticEditorWindow> mCosmeticEditorWindow;
// std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
// std::shared_ptr<CollisionViewerWindow> mCollisionViewerWindow;
// std::shared_ptr<EventLogWindow> mEventLogWindow;
// std::shared_ptr<DLViewerWindow> mDLViewerWindow;
// std::shared_ptr<MessageViewerWindow> mMessageViewerWindow;
// std::shared_ptr<AudioEditor> mAudioEditorWindow;
std::shared_ptr<LighthouseMenu> mLighthouseMenu;
std::shared_ptr<Notification::Window> mNotificationWindow;
std::shared_ptr<Rando::CheckTracker::CheckTrackerWindow> mRandoCheckTrackerWindow;
std::shared_ptr<Rando::CheckTracker::SettingsWindow> mRandoCheckTrackerSettingsWindow;
// std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
// std::shared_ptr<ItemTrackerSettingsWindow> mItemTrackerSettingsWindow;
std::shared_ptr<DisplayOverlayWindow> mDisplayOverlayWindow;
// std::shared_ptr<TimesplitsWindow> mTimesplitsWindow;
// std::shared_ptr<TimesplitsSettingsWindow> mTimesplitsSettingsWindow;
std::shared_ptr<InputViewer> mInputViewer;
std::shared_ptr<InputViewerSettingsWindow> mInputViewerSettings;
std::shared_ptr<EggAimCrosshairWindow> mEggAimCrosshair;
std::shared_ptr<LighthouseModalWindow> mModalWindow;
std::shared_ptr<EventDebuggerWindow> mEventDebuggerWindow;
std::shared_ptr<OcclusionDebugWindow> mOcclusionDebugWindow;
std::shared_ptr<AnchorRoomWindow> mAnchorRoomWindow;

UIWidgets::Colors GetMenuThemeColor() {
    return mLighthouseMenu->GetMenuThemeColor();
}

void SetupMenu() {
    auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();
    mLighthouseMenu = std::make_shared<LighthouseGui::LighthouseMenu>(CVAR_WINDOW("Menu"), "Port Menu");
    gui->SetMenu(mLighthouseMenu);

    mModalWindow = std::make_shared<LighthouseModalWindow>(CVAR_WINDOW("ModalWindow"), "Modal Window");
    gui->AddGuiWindow(mModalWindow);
    mModalWindow->Show();
}

void SetupGuiElements() {
    auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();

    auto& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(4.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.Colors[ImGuiCol_MenuBarBg] = UIWidgets::ColorValues.at(UIWidgets::Colors::DarkGray);

    mStatsWindow = gui->GetGuiWindow("Stats");
    if (mStatsWindow == nullptr) {
        SPDLOG_ERROR("Could not find stats window");
    }

    mConsoleWindow = gui->GetGuiWindow("Console");
    if (mConsoleWindow == nullptr) {
        SPDLOG_ERROR("Could not find console window");
    }

    // mGfxDebuggerWindow = gui->GetGuiWindow("GfxDebuggerWindow");
    // if (mGfxDebuggerWindow == nullptr) {
    //     SPDLOG_ERROR("Could not find input GfxDebuggerWindow");
    // }

    mInputEditorWindow =
        std::make_shared<LighthouseInputEditorWindow>(CVAR_WINDOW("ControllerConfiguration"), "Configure Controller");
    gui->AddGuiWindow(mInputEditorWindow);

    mGamepadMapperWindow =
        std::make_shared<Mapper::MapperWindow>(CVAR_WINDOW("GamepadMapper"), "Gamepad Mapper", ImVec2(1280, 820));
    gui->AddGuiWindow(mGamepadMapperWindow);

    mModMenuWindow = std::make_shared<LighthouseModMenuWindow>(CVAR_WINDOW("ModMenu"), "Mod Menu");
    gui->AddGuiWindow(mModMenuWindow);

    mRomhackMenuWindow = std::make_shared<LighthouseRomhackMenuWindow>(CVAR_WINDOW("RomhackMenu"), "Romhack Menu");
    gui->AddGuiWindow(mRomhackMenuWindow);

    // mHookDebuggerWindow =
    //     std::make_shared<HookDebuggerWindow>("gWindows.HookDebugger", "Hook Debugger", ImVec2(480, 600));
    // gui->AddGuiWindow(mHookDebuggerWindow);

    mSaveEditorWindow = std::make_shared<SaveEditorWindow>("gWindows.SaveEditor", "Save Editor", ImVec2(480, 600));
    gui->AddGuiWindow(mSaveEditorWindow);

    mGameplayToolsWindow =
        std::make_shared<GameplayToolsWindow>("gWindows.GameplayTools", "Gameplay Tools", ImVec2(480, 600));
    gui->AddGuiWindow(mGameplayToolsWindow);

    // mHudEditorWindow = std::make_shared<HudEditorWindow>("gWindows.HudEditor", "HUD Editor", ImVec2(480, 600));
    // gui->AddGuiWindow(mHudEditorWindow);

    // mCosmeticEditorWindow =
    //     std::make_shared<CosmeticEditorWindow>("gWindows.CosmeticEditor", "Cosmetic Editor", ImVec2(480, 600));
    // gui->AddGuiWindow(mCosmeticEditorWindow);

    // mActorViewerWindow = std::make_shared<ActorViewerWindow>("gWindows.ActorViewer", "Actor Viewer", ImVec2(520,
    // 600)); gui->AddGuiWindow(mActorViewerWindow);

    // mCollisionViewerWindow =
    //     std::make_shared<CollisionViewerWindow>("gWindows.CollisionViewer", "Collision Viewer", ImVec2(390, 475));
    // gui->AddGuiWindow(mCollisionViewerWindow);

    // mEventLogWindow = std::make_shared<EventLogWindow>("gWindows.EventLog", "Event Log", ImVec2(520, 600));
    // gui->AddGuiWindow(mEventLogWindow);

    // mDLViewerWindow = std::make_shared<DLViewerWindow>("gWindows.DLViewer", "DL Viewer", ImVec2(520, 600));
    // gui->AddGuiWindow(mDLViewerWindow);
    // mMessageViewerWindow =
    //     std::make_shared<MessageViewerWindow>("gWindows.MessageViewer", "Message Viewer", ImVec2(520, 600));
    // gui->AddGuiWindow(mMessageViewerWindow);

    // mAudioEditorWindow = std::make_shared<AudioEditor>("gWindows.AudioEditor", "Audio Editor", ImVec2(520, 600));
    // gui->AddGuiWindow(mAudioEditorWindow);

    // mItemTrackerWindow = std::make_shared<ItemTrackerWindow>("gWindows.ItemTracker", "Item Tracker");
    // gui->AddGuiWindow(mItemTrackerWindow);

    // mItemTrackerSettingsWindow = std::make_shared<ItemTrackerSettingsWindow>("gWindows.ItemTrackerSettings",
    //                                                                          "Item Tracker Settings", ImVec2(800,
    //                                                                          400));
    // gui->AddGuiWindow(mItemTrackerSettingsWindow);

    mDisplayOverlayWindow = std::make_shared<DisplayOverlayWindow>("gWindows.DisplayOverlay", "Display Overlay");
    gui->AddGuiWindow(mDisplayOverlayWindow);

    // mTimesplitsWindow = std::make_shared<TimesplitsWindow>("gWindows.Timesplits", "Time Splits Window");
    // gui->AddGuiWindow(mTimesplitsWindow);

    // mTimesplitsSettingsWindow = std::make_shared<TimesplitsSettingsWindow>(
    //     "gWindows.Timesplits.Settings", "Time Splits Settings Window", ImVec2(567, 97));
    // gui->AddGuiWindow(mTimesplitsSettingsWindow);

    mNotificationWindow = std::make_shared<Notification::Window>("gWindows.Notifications", "Notifications Window");
    gui->AddGuiWindow(mNotificationWindow);
    mNotificationWindow->Show();

    mRandoCheckTrackerWindow = std::make_shared<Rando::CheckTracker::CheckTrackerWindow>(
        "gWindows.CheckTracker", "Check Tracker", ImVec2(375, 460));
    gui->AddGuiWindow(mRandoCheckTrackerWindow);
    mEggAimCrosshair = std::make_shared<EggAimCrosshairWindow>("gWindows.EggAimCrosshair", "Egg Aim Crosshair");
    gui->AddGuiWindow(mEggAimCrosshair);
    mEggAimCrosshair->Show();

    mRandoCheckTrackerSettingsWindow = std::make_shared<Rando::CheckTracker::SettingsWindow>(
        "gWindows.CheckTrackerSettings", "Check Tracker Settings");
    gui->AddGuiWindow(mRandoCheckTrackerSettingsWindow);

    mInputViewer = std::make_shared<InputViewer>(CVAR_WINDOW("InputViewer"), "Input Viewer");
    gui->AddGuiWindow(mInputViewer);

    mInputViewerSettings = std::make_shared<InputViewerSettingsWindow>(CVAR_WINDOW("InputViewerSettings"),
                                                                       "Input Viewer Settings", ImVec2(500, 525));
    gui->AddGuiWindow(mInputViewerSettings);

    mEventDebuggerWindow = std::make_shared<EventDebuggerWindow>(CVAR_WINDOW("EventDebugger"), "Event Debugger");
    gui->AddGuiWindow(mEventDebuggerWindow);

    mOcclusionDebugWindow = std::make_shared<OcclusionDebugWindow>(CVAR_WINDOW("OcclusionDebug"), "Occlusion Debugger");
    gui->AddGuiWindow(mOcclusionDebugWindow);

    mAnchorRoomWindow = std::make_shared<AnchorRoomWindow>(CVAR_WINDOW("AnchorRoom"), "Anchor Room");
    gui->AddGuiWindow(mAnchorRoomWindow);
}

void Destroy() {
    auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();

    gui->RemoveAllGuiWindows();
    mLighthouseMenu = nullptr;
    mModalWindow = nullptr;
    mStatsWindow = nullptr;
    mConsoleWindow = nullptr;
    mGfxDebuggerWindow = nullptr;
    mInputEditorWindow = nullptr;
    mGamepadMapperWindow = nullptr;
    // mCollisionViewerWindow = nullptr;
    // mEventLogWindow = nullptr;
    mNotificationWindow = nullptr;
    mRandoCheckTrackerWindow = nullptr;
    mRandoCheckTrackerSettingsWindow = nullptr;

    // mHookDebuggerWindow = nullptr;
    mSaveEditorWindow = nullptr;
    mGameplayToolsWindow = nullptr;
    // mHudEditorWindow = nullptr;
    // mCosmeticEditorWindow = nullptr;
    // mActorViewerWindow = nullptr;
    // mDLViewerWindow = nullptr;
    // mMessageViewerWindow = nullptr;
    // mAudioEditorWindow = nullptr;
    // mItemTrackerWindow = nullptr;
    // mItemTrackerSettingsWindow = nullptr;
    mDisplayOverlayWindow = nullptr;
    mInputViewer = nullptr;
    mInputViewerSettings = nullptr;
    mEggAimCrosshair = nullptr;
    mEventDebuggerWindow = nullptr;
    mOcclusionDebugWindow = nullptr;
    mAnchorRoomWindow = nullptr;
}

void RegisterPopup(std::string title, std::string message, std::string button1, std::string button2,
                   std::function<void()> button1callback, std::function<void()> button2callback) {
    mModalWindow->RegisterPopup(title, message, button1, button2, button1callback, button2callback);
}

size_t PopupsQueued() {
    return mModalWindow->PopupsQueued();
}

} // namespace LighthouseGui
