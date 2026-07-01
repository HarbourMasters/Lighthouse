#ifndef PORT_ORBIT_CAMERA_CORE_H
#define PORT_ORBIT_CAMERA_CORE_H

// Shared right-stick orbit camera core.
//
// Both the modern control scheme's yaw orbit and the Free Look enhancement are
// configurations of the same machinery: capture the current camera, drive yaw
// (and optionally pitch) from the right stick, track the vanilla zoom-level
// distance, resolve geometry collision, smooth, and aim back at the player.
//
// The two only differ in policy (when to enter/hold/hand back, how the stick is
// read, whether pitch is live) and in a couple of tuning values. Each owns an
// OrbitCamera instance and supplies those via the config fields below; the core
// owns the per-frame math so the behavior stays identical between them.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // // Config (set by the owner)
    int stateId;    // dynamic camera state this orbit runs as
    int allowPitch; // 1: stick Y drives pitch and feeds the vertical offset.
                    // 0: pitch is locked flat and height tracks the vanilla target.
    float minPitch; // pitch clamp, degrees (only used when allowPitch)
    float maxPitch;
    float smoothRate; // position smoothing rate, 1/sec

    // Runtime state
    int active;
    int justEntered;
    int smoothValid;
    int aimValid; // aimCenterY has been seeded
    float yaw;
    float pitch;
    float distance;   // spherical radius; tracks the vanilla zoom-level target
    float height;     // absolute Y, only used when !allowPitch
    float aimCenterY; // smoothed Y of the look-at target (de-steps stairs)
    float smoothPos[3];
} OrbitCamera;

// Seed yaw/pitch/distance(/height) from wherever the live camera currently sits,
// so the orbit starts without a jump.
void OrbitCamera_Capture(OrbitCamera* c);

// Capture and take ownership of the camera state.
void OrbitCamera_Enter(OrbitCamera* c);

// Release the camera state back to the normal follow camera (state 0xB).
void OrbitCamera_Exit(OrbitCamera* c);

// Advance one frame.
void OrbitCamera_Update(OrbitCamera* c, float yawDelta, float pitchDelta);

#ifdef __cplusplus
}
#endif

#endif // PORT_ORBIT_CAMERA_CORE_H
