#pragma once

#ifdef __cplusplus
#include <unordered_map>
#include <libultraship/libultra/gbi.h>
#include <fast/types.h>


void FrameInterpolation_RecordFrame(Gfx* commands);
std::unordered_map<Mtx*, MtxF> FrameInterpolation_Interpolate(float t);

#endif
