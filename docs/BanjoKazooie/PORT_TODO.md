# PORT TODO

## Lighthouse.o2r

### Add a game selector
Lighthouse supports romhacks created with Banjo's Backpack, if a config.yml entry is added. The lighthouse.o2r file can therefore have a game selector to allow the user to choose which o2r file to launch.

## Vanilla Issues

### Lair Continuity Bug
Exiting the lair out to spiral mountain continues lair music. This is all done in audio_instruments.c

### Aspect 4:3 top/bottom frustum draw distance bug
In 4:3 mode many things are being culled at 100% draw distance; needs same top/bottom frustum checks as widescreen since we go from 292x216 to 320x240 at 4:3.

### PAL crashes
PAL o2r has some issues with rendering accent characters which cause flickering in zoombox dialog. There is also a crash to do with speaker sprites.

### MacOS Lag
On Metal, framebuffers (falling jiggy transition, pause menu, bottles bonus and sns) have heavy lag. OpenGL path works fine.

## Enhancement Opportunities

### GAMEPLAY: Note & Jinjo Tracking
A staple of the Xbox360 Arcade version of the game tracks music notes and jinjos across worlds; in N64 world states are reset.

### GAMEPLAY: Other World State Tracking
An enhancement could also track other world states that get reset such as Clanker being raised above water in Clanker's Cavern, the Sandcastle moat being drained in TTC, etc.

### GENERAL: Save File Conversion
Allow users to drag and drop emulator save files onto the port window to have them converted to the modern json format. Emulator saves will need detection for romhacks and then be sorted into their respective folders.