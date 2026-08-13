#pragma once

typedef enum {
    BK_VER_US_10 = 0x0693BFA4,
    BK_VER_US_11 = 0xAC5975CD,
    BK_VER_PAL = 0xB1CC3F73,
    BK_VER_JP = 0x20D56851,
} BKVersion;

#ifdef __cplusplus
#include <vector>
#include <SDL2/SDL.h>
#include <fast/interpreter.h>
#include "ship/Context.h"
#include "ship/window/gui/GameOverlay.h"

#ifndef IDYES
#define IDYES 6
#endif
#ifndef IDNO
#define IDNO 7
#endif

class GameEngine {
public:
    static GameEngine* Instance;

    ImFont* fontStandard;
    ImFont* fontStandardLarger;
    ImFont* fontStandardLargest;
    ImFont* fontMono;
    ImFont* fontMonoLarger;
    ImFont* fontMonoLargest;

    Ship::Context* context;

    GameEngine();
    void StartFrame() const;
    static void Create(int argc, char* argv[]);
    static void AudioInit();
    void FinishInit();
    void RunExtract(int argc, char* argv[]);
    // Render a GUI-only frame (no game tick). Used to keep the ImGui progress
    // modal live while an inline mod extraction runs on a worker thread.
    void RenderGuiFrame() const;

    // Mod changes only bind at process start, so applying them needs a full
    // relaunch rather than the soft in-game reset. UI requests it, then closes
    // the window; the main loop re-execs the app after teardown completes.
    static bool sRelaunchRequested;
    static void RequestRelaunch() {
        sRelaunchRequested = true;
    }
    static void RelaunchIfRequested(int argc, char* argv[]);
    static void RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements,
                            size_t frameCount);
    static void Destroy();
    static uint32_t GetInterpolationFPS();
    static bool IsInterpolationEnabled();
    static void SetInterpolationRecorded(bool recorded);
    static void ProcessGfxCommands(Gfx* commands);
    static ImFont* CreateFontWithSize(float size, std::string fontPath);
    static void ScaleImGui();
};

Fast::Interpreter* GameEngine_GetInterpreter();
#define memallocn(type, n) (type*)GameEngine_Malloc(sizeof(type) * n)
#define memalloc(type) memallocn(type, 1)

extern "C" {
#else
#include <stdint.h>
#define memalloc(size) GameEngine_Malloc(size)
#endif

void* GameEngine_Malloc(size_t size);
void GameEngine_Free(void* ptr);
float GameEngine_GetAspectRatio();
float OTRGetDimensionFromLeftEdge(float v);
float OTRGetDimensionFromRightEdge(float v);
int16_t OTRGetRectDimensionFromLeftEdge(float v);
int16_t OTRGetRectDimensionFromRightEdge(float v);
uint32_t OTRGetGameRenderWidth();
uint32_t OTRGetGameRenderHeight();

#ifdef __cplusplus
}
#endif