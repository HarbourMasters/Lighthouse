#pragma once

// Geometry-cull dispatch.
//
// BK's map model conditionally culls chunks of level geometry via a few geo commands
// (camera-area portal, LOD distance band, frustum sphere). render.c routes each of those
// decisions through port_geoCullDraw, which fires the OnGeoCull event so port-side
// listeners — the occlusion debugger and the draw-distance enhancement — can force a chunk
// to draw. When no listener is active the call is a cheap no-op, so normal play pays
// nothing.

#ifdef __cplusplus
extern "C" {
#endif

// Conditional cull command kinds.
#define OCCLUSION_CMD_CAMERA 0 // geoCmd_CAMERA: gated on which camera-area box the camera is in
#define OCCLUSION_CMD_LOD 1    // geoCmd_LOD: gated on distance band [min, max]
#define OCCLUSION_CMD_UNKE 2   // geoCmd_UnkE: gated on a bounding sphere passing the view frustum

// Consumer bits for GeoCull_SetConsumer; the event only fires while at least one is set.
#define GEOCULL_CONSUMER_DEBUG 1
#define GEOCULL_CONSUMER_ENHANCEMENT 2

// Called from each conditional cull command in render.c. `cmd`/`modelBin` give the command's
// stable byte offset; `drawnVanilla` is the unmodified recurse decision. areaIds/areaCount
// apply to CAMERA only (NULL/0 otherwise); detail0/detail1 are kind-specific extras (CAMERA:
// flags; LOD: min,max). Returns the final draw decision (1 = draw).
int port_geoCullDraw(int type, const void* cmd, const void* modelBin, int drawnVanilla, const unsigned char* areaIds,
                     int areaCount, int detail0, int detail1);

// Listeners enable/disable themselves as consumers so the per-command event firing can be
// skipped entirely when nothing is listening.
void GeoCull_SetConsumer(int consumerBit, int active);

#ifdef __cplusplus
}
#endif
