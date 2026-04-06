# PORT TODO

## Lighthouse.o2r

### Add a game selector
Lighthouse supports romhacks created with Banjo's Backpack, if a config.yml entry is added. The lighthouse.o2r file can therefore have a game selector to allow the user to choose which o2r file to launch.

## Vanilla Issues

### Texture Seams
Many textures are broken into sections and have seams. These seams are built into the models themselves and are a result of the developers not accounting for bilerp filtering. In order to have seamless transitions between bilerp filtered textures, you must duplicate the first row of the previous texture in the next texture. In other words, the textures themselves are missing information due to a developer oversight.

### MacOS Lag
On Metal, framebuffers (falling jiggy transition, pause menu, bottles bonus and sns) have heavy lag. OpenGL path works fine.

## Enhancement Opportunities

### GAMEPLAY: Note & Jinjo Tracking
A staple of the Xbox360 Arcade version of the game tracks music notes and jinjos across worlds; in N64 world states are reset.

### GAMEPLAY: Other World State Tracking
An enhancement could also track other world states that get reset such as Clanker being raised above water in Clanker's Cavern, the Sandcastle moat being drained in TTC, etc.

### GENERAL: Save File Conversion
Allow users to drag and drop emulator save files onto the port window to have them converted to the modern json format. Emulator saves will need detection for romhacks and then be sorted into their respective folders.



