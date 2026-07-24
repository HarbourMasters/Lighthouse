#ifndef PORT_ORBIT_CAMERA_CORE_H
#define PORT_ORBIT_CAMERA_CORE_H

// Shared right-stick orbit camera core: capture the live camera, drive yaw (and optionally pitch)
// from the stick, track the vanilla zoom distance, resolve collision, smooth, and aim at the player.
// The modern-scheme yaw orbit and Free Look are the same machinery with different policy and tuning.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // Config (set by the owner)
    int stateId;    // dynamic camera state this orbit runs as
    int allowPitch; // 1: stick Y drives pitch; 0: pitch locked flat, height tracks the vanilla target
    float minPitch; // pitch clamp, degrees (only used when allowPitch)
    float maxPitch;
    float smoothRate; // position smoothing rate, 1/sec

    // Runtime state
    int active;
    int justEntered;
    int smoothValid;
    int aimValid;
    float yaw;
    float pitch;
    float distance;   // spherical radius; tracks the vanilla zoom-level target
    float height;     // absolute Y, only used when !allowPitch
    float aimCenterY; // smoothed Y of the look-at target
    float smoothPos[3];
    float collisionOffset[3]; // smoothed collision correction, kept separate from the follow motion
    float smoothRot[3];       // damped look-at rotation
    int rotValid;
} OrbitCamera;

// Seed yaw/pitch/distance(/height) from the live camera so the orbit starts without a jump.
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
