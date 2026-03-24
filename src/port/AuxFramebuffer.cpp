#include <cstring>
#include <unordered_map>

#include <fast/interpreter.h>
#include "Engine.h"
#include "ShipUtils.h"

struct AuxColorImage {
    int fbId;
    uint32_t width, height;  // native dimensions
    uint32_t curFbW, curFbH; // current FBO dimensions (avoids per-frame resize)
};
static std::unordered_map<uintptr_t, AuxColorImage> sAuxColorImages;
static int sActiveAuxFb = -1;

static void auxColorImageCallback(void* oldAddr, void* newAddr) {
    bool oldIsAux = sAuxColorImages.count((uintptr_t)oldAddr) > 0;
    bool newIsAux = sAuxColorImages.count((uintptr_t)newAddr) > 0;

    // Track whether we need a GPU depth+color clear after the readback.
    bool needPostReadbackClear = false;
    if (oldIsAux && !newIsAux && sActiveAuxFb >= 0) {
        auto interp = GameEngine_GetInterpreter();
        if (interp && interp->mRapi && (uintptr_t)newAddr == (uintptr_t)interp->mRdp->z_buf_address) {
            needPostReadbackClear = true;
        }
    }

    if (newIsAux && sActiveAuxFb < 0) {
        sActiveAuxFb = sAuxColorImages[(uintptr_t)newAddr].fbId;
    }

    if (!oldIsAux || sActiveAuxFb < 0) {
        return;
    }

    auto interpreter = GameEngine_GetInterpreter();
    if (!interpreter || !interpreter->mRapi) {
        return;
    }
    auto& aux = sAuxColorImages[(uintptr_t)oldAddr];
    auto* rapi = interpreter->mRapi;

    int mainFb = interpreter->mRendersToFb ? interpreter->mGameFb : 0;
    uint32_t gpuW = 0, gpuH = 0;
    interpreter->GetCurDimensions(&gpuW, &gpuH);

    float ratioX = (float)gpuW / interpreter->mNativeDimensions.width;
    float ratioY = (float)gpuH / interpreter->mNativeDimensions.height;
    uint32_t srcW = (uint32_t)(aux.width * ratioX);
    uint32_t srcH = (uint32_t)(aux.height * ratioY);

    // OpenGL's glBlitFramebuffer scales during copy; DX11's CopySubresourceRegion is 1:1.
    const char* apiName = rapi->GetName();
    bool isOpenGL = (apiName && strstr(apiName, "OpenGL") != nullptr);

    if (isOpenGL) {
        if (aux.curFbW != aux.width || aux.curFbH != aux.height) {
            rapi->UpdateFramebufferParameters(aux.fbId, aux.width, aux.height, 1, true, true, true, false);
            aux.curFbW = aux.width;
            aux.curFbH = aux.height;
        }
        rapi->CopyFramebuffer(aux.fbId, mainFb, 0, 0, srcW, srcH, 0, 0, aux.width, aux.height);
        rapi->ReadFramebufferToCPU(aux.fbId, aux.width, aux.height, (uint16_t*)oldAddr);
    } else {
        if (aux.curFbW != srcW || aux.curFbH != srcH) {
            rapi->UpdateFramebufferParameters(aux.fbId, srcW, srcH, 1, true, true, true, false);
            aux.curFbW = srcW;
            aux.curFbH = srcH;
        }
        rapi->CopyFramebuffer(aux.fbId, mainFb, 0, 0, srcW, srcH, 0, 0, srcW, srcH);
        rapi->ReadFramebufferToCPU(aux.fbId, aux.width, aux.height, (uint16_t*)oldAddr);
    }

    // Byte-swap to big-endian (N64 pixel convention)
    uint16_t* pixels = (uint16_t*)oldAddr;
    uint32_t pixelCount = aux.width * aux.height;
    for (uint32_t i = 0; i < pixelCount; i++) {
        pixels[i] = (pixels[i] >> 8) | (pixels[i] << 8);
    }

    // Invalidate texture cache entries within the buffer range
    interpreter->TextureCacheDeleteRange((const uint8_t*)oldAddr, pixelCount * sizeof(uint16_t));
    sActiveAuxFb = -1;

    // Clear GPU depth + color AFTER readback.
    if (needPostReadbackClear) {
        interpreter->Flush();
        rapi->ClearFramebuffer(true, true);
    }
}

extern "C" void port_registerAuxColorImage(void* cpuAddr, uint32_t width, uint32_t height) {
    uintptr_t key = (uintptr_t)cpuAddr;
    if (sAuxColorImages.count(key)) {
        return;
    }

    auto interpreter = GameEngine_GetInterpreter();
    if (!interpreter || !interpreter->mRapi) {
        return;
    }

    int fb = interpreter->mRapi->CreateFramebuffer();
    interpreter->mRapi->UpdateFramebufferParameters(fb, width, height, 1, true, true, true, false);
    interpreter->mFrameBuffers[fb] = { width, height, width, height, width, height, false };
    sAuxColorImages[key] = { fb, width, height, width, height };

    // Install callback if not already set.
    interpreter->SetColorImageChangeCallback(auxColorImageCallback);
}

extern "C" void port_unregisterAuxColorImage(void* cpuAddr) {
    uintptr_t key = (uintptr_t)cpuAddr;
    auto it = sAuxColorImages.find(key);
    if (it == sAuxColorImages.end()) {
        return;
    }

    auto interpreter = GameEngine_GetInterpreter();
    if (interpreter) {
        interpreter->mFrameBuffers.erase(it->second.fbId);
    }
    if (sActiveAuxFb == it->second.fbId) {
        sActiveAuxFb = -1;
    }
    sAuxColorImages.erase(it);
}
