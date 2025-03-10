#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "libultraship/libultraship.h"
#include <memory>

typedef enum {

} BK64Version;

#ifndef IDYES
#define IDYES 6
#endif
#ifndef IDNO
#define IDNO 7
#endif

class GameEngine {
  public:
    static GameEngine* Instance;

    std::shared_ptr<Ship::Context> context;

    GameEngine();
    void StartFrame() const;
    static bool GenAssetFile();
    static void Create();
    static void HandleAudioThread();
    static void StartAudioFrame();
    static void EndAudioFrame();
    static void AudioInit();
    static void AudioExit();
    static void RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements);
    static void Destroy();
    static uint32_t GetInterpolationFPS();
    static void ProcessGfxCommands(Gfx* commands);

    static int ShowYesNoBox(const char* title, const char* box);
    static void ShowMessage(const char* title, const char* message, SDL_MessageBoxFlags type = SDL_MESSAGEBOX_ERROR);
    static bool HasVersion(BK64Version ver);
};

bool GameEngine_HasVersion(BK64Version ver);
void GameEngine_ProcessGfxCommands(Gfx* commands);
float GameEngine_GetAspectRatio();
uint8_t GameEngine_OTRSigCheck(char* imgData);
uint32_t OTRGetCurrentWidth(void);
uint32_t OTRGetCurrentHeight(void);
float OTRGetAspectRatio(void);
float OTRGetHUDAspectRatio();
int32_t OTRConvertHUDXToScreenX(int32_t v);
float OTRGetDimensionFromLeftEdge(float v);
float OTRGetDimensionFromRightEdge(float v);
int16_t OTRGetRectDimensionFromLeftEdge(float v);
int16_t OTRGetRectDimensionFromRightEdge(float v);
float OTRGetDimensionFromLeftEdgeForcedAspect(float v, float aspectRatio);
float OTRGetDimensionFromRightEdgeForcedAspect(float v, float aspectRatio);
int16_t OTRGetRectDimensionFromLeftEdgeForcedAspect(float v, float aspectRatio);
int16_t OTRGetRectDimensionFromRightEdgeForcedAspect(float v, float aspectRatio);
float OTRGetDimensionFromLeftEdgeOverride(float v);
float OTRGetDimensionFromRightEdgeOverride(float v);
int16_t OTRGetRectDimensionFromLeftEdgeOverride(float v);
int16_t OTRGetRectDimensionFromRightEdgeOverride(float v);
uint32_t OTRGetGameRenderWidth();
uint32_t OTRGetGameRenderHeight();
void* GameEngine_Malloc(size_t size);
void GameEngine_GetTextureInfo(const char* path, int32_t* width, int32_t* height, float* scale, bool* custom);
#define memalloc(size) GameEngine_Malloc(size)

#endif // GAMEENGINE_H