# PORT TODO

## Lighthouse.o2r

### Add a game selector
Lighthouse supports romhacks created with Banjo's Backpack, if a config.yml entry is added. The lighthouse.o2r file can therefore have a game selector to allow the user to choose which o2r file to launch.

## Vanilla Issues

### Texture Seams
Many textures are broken into sections and have seams. These seams are built into the models themselves and are a result of the developers not accounting for bilerp filtering. In order to have seamless transitions between bilerp filtered textures, you must duplicate the first row of the previous texture in the next texture. In other words, the textures themselves are missing information due to a developer oversight.

### Texture Oddities
In romhacks (Jiggies of Time notably) there are texture oddities. In particular, in the map TTC_LOBBY (which is Link's House in the hack), the door to go outside is almost entirely black with white/yellow spots, instead of being white with varying brightness. Also, the scarf hanging on the wall has a broken palette or stretching instead of being the mostly red that appears on Retroarch.

There may be a clue in Freezeezy Peak's lobby in vanilla, where the door texture is incorrectly stretched. Check tcrf website for info where this was corrected on the xbox arcade version.

### Widescreen cutscene angles
In widescreen only, some cutscenes will angle the camera in a way that exposes the skybox outside of model geometry bounds. BanjoRecomp solves this by adding pillarboxing in specific scenes that do this. Xbox360 arcade changes the camera position instead. Choose a path.

## Enhancement Opportunities

### Furnace Fun Game Over dialog
When stepping on a skull panel in GFF, dialog warns about a game over if you fail -- but it triggers if you have one life left, not zero lives.

### Grunty Defeated Flag Offset
In v1.1, the Grunty Defeated flag flip was moved to trigger later in the sequence, since the player could still die after they'd fired an egg to free Jinjonator.

### Void-Out Game Over
If the player triggers the void out function with zero lives, they trigger a game over despite not actually losing a life in void outs.

### Note & Jinjo Tracking
A staple of the Xbox360 Arcade version of the game tracks music notes and jinjos across worlds; in N64 world states are reset.

### Other World State Tracking
An enhancement could also track other world states that get reset such as Clanker being raised above water in Clanker's Cavern, the Sandcastle moat being drained in TTC, etc.

### GV Water Pyramid Timer
The water pyramid's top hatch door opens via switch and is timed for 20 seconds (+4 for cutscene of it opening). An enhancement could extend the timer by an additional 4 seconds to make things easier for slower users.

### Skip Jiggy Dance
On collecting a jiggy, Banjo will do a dance. On collecting all 10 in a world, he will do a different dance. An enhancement could skip these dances which stop the player briefly, but we will need to be careful that we don't introduce oddities like https://github.com/BanjoRecomp/BanjoRecomp/issues/229

### Stop N' Swop
An enhancement could activate the Stop N' Swop cheats at 100% game completion as a reward.

### Missable Mumbo Tokens
- In GV water pyramid a token cannot be collecvted after the water is drained. Could change its Y position to be reachable from the ground after water is drained.
- In MMM two tokens share the same actor id and therefore one will despawn when the other is collected. This is already marked as `#ifdef PORT_FIX`.
- In CCW (Spring) two tokens share the same actor id but are on the same map, so collecting one and leaving the map will despawn the other. This is also marked.

### Gruntilda's Lair Audio
Gruntilda's Lair has several audio tracks that fade to different instruments when in proximity of a world's puzzle podium or entrypoint. These are actually different audio tracks due to instrument data, but the beginning bits of each track are the same. An enhancement could track the position of tracks in the Lair and start the next track at the same position.

### Save File Conversion
Allow users to drag and drop emulator save files onto the port window to have them converted to the modern json format. Emulator saves will need detection for romhacks and then be sorted into their respective folders.



