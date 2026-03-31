# PORT TODO

## Decomp code

### Documentation
The BK decomp at https://gitlab.com/banjo.decomp/banjo-kazooie has received a lot of documentation and corrections since it was taken into Lighthouse. Lighthouse commit history shows ~late 2024 for decomp, which misses a lot of useful documentation adds that came later like enums.h. We should bring more documentation over from decomp.

## Lighthouse.o2r

### Add a game selector
Lighthouse supports romhacks created with Banjo's Backpack, if a config.yml entry is added. The lighthouse.o2r file can therefore have a game selector to allow the user to choose which o2r file to launch.

## Vanilla Issues

### Texture Seams
Many textures are broken into sections and have seams. These seams are built into the models themselves and are a result of the developers not accounting for bilerp filtering. In order to have seamless transitions between bilerp filtered textures, you must duplicate the first row of the previous texture in the next texture. In other words, the textures themselves are missing information due to a developer oversight.

### Shadow Texture Cache Bug
Banjo's shadow texture changes appearance when the GPU texture cache is flushed. In some areas of romhack maps, the shadow renders as a solid dark square instead of a proper circular shadow. Moving to a nearby location (same map, just a few steps away) causes the shadow to render fine. Flushing the cache changes the shadow texture when it's a square, suggesting a cache key collision or stale RDP state at first decode time. Textures can be converted to bmp from bk-jot.o2r/assets.

### Widescreen cutscene angles
In widescreen only, some cutscenes will angle the camera in a way that exposes the skybox outside of model geometry bounds. We want to adjust the camera yaw for the specific nodes that cause this. Known cases:
- Nintendo intro concert: when Banjo looks at Tooty playing flute, camera needs to angle right.
- MM Bottles beak buster molehill: static camera during dialogue needs to angle right.

**TODO:** Add a dev tools ImGui debug box that logs static camera position changes (map ID, camera node index, position, rotation). Only log when camera type is CAMERA_TYPE_3_STATIC and when the node changes. Use the logged node indices to build a correction table in `ncStaticCamera_setToNode`.

### MacOS Lag
On Metal, framebuffers (falling jiggy transition, pause menu, bottles bonus and sns) have heavy lag. OpenGL path works fine.

## Enhancement Opportunities

### GAMEPLAY: Note & Jinjo Tracking
A staple of the Xbox360 Arcade version of the game tracks music notes and jinjos across worlds; in N64 world states are reset.

### GAMEPLAY: Other World State Tracking
An enhancement could also track other world states that get reset such as Clanker being raised above water in Clanker's Cavern, the Sandcastle moat being drained in TTC, etc.

### TEST: Gruntilda's Lair Audio
Gruntilda's Lair has several audio tracks that fade to different instruments when in proximity of a world's puzzle podium or entrypoint. Each floor is actually a different audio track due to instrument data, but the beginning bits of each track are the same. An enhancement could track the position of tracks in the Lair and start the next track at the same position, giving the illusion of smooth flow.

### GENERAL: Save File Conversion
Allow users to drag and drop emulator save files onto the port window to have them converted to the modern json format. Emulator saves will need detection for romhacks and then be sorted into their respective folders.



