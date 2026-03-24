# Romhack Support

Lighthouse supports BK romhacks built with **Banjo's Backpack (BB)**. All romhack configuration is auto-detected during extraction — no manual setup beyond providing the ROM.

## Adding a Romhack

1. Add the ROM's SHA-1 hash to `config.yml` with a unique output name:
   ```yaml
   a9f62c403a21ae9d9a1bbf9e92d752fc2cf85c2b:
     name: Banjo-Kazooie - Jiggies of Time
     path: assets/yaml/us/rev0
     config:
       gbi: F3DEX_BK64
       sort: OFFSET
       logging: WARN
       textures: ADDITIONAL_DEFINES
       include_autogen: true
       output:
         binary: bk-jot.o2r
   ```
2. Add the o2r filename to `sRomArchives` in `src/port/Engine.cpp`.
3. Place the romhack ROM alongside the vanilla ROM and run the extractor.

All BB settings (scene assignments, music, skyboxes, item limits, puzzle costs, etc.) are auto-detected from the ROM and stored in the o2r. No code changes needed per romhack.

## Save Files

Romhack saves are stored separately from vanilla saves. The save directory is based on the o2r name:

- Vanilla: `saves/file1.json`, `saves/file2.json`, `saves/file3.json`
- Romhack: `saves/bk-jot/file1.json`, `saves/bk-jot/file2.json`, etc.

This prevents romhack and vanilla save data from interfering with each other.

## What Gets Auto-Detected

BB romhacks can customize many game settings through the General Settings editor. Lighthouse extracts all of them:

| BB Tab | What It Controls |
|--------|-----------------|
| Start Level | Which map loads on new game |
| Level Names | Display names in pause menu |
| Scene Assignment | Which level each map belongs to |
| Music Assignment | Music tracks per map |
| Skyboxes | Skybox models, scales, rotation per scene (3 layers) |
| Warps | All warp destinations (MIPS instruction scanning), return-to-lair, Banjo's House/Lair warps |
| Max Items | Eggs, feathers, notes, jiggies, honeycombs (normal + Cheato) |
| Unlockable Requirements | Note door thresholds, jiggy puzzle costs, mumbo costs |

## N64-Only Changes (Ignored)

BB also applies overlay relocation (the "Globalizer") and anti-tamper patches. These are N64-specific and have no effect on the port:

- Boot stub DMA code for globalized overlays
- JAL/LUI instruction address relocation
- Dispatch table NOPs for unused overlay slots

## Enhancements

Some port enhancements are automatically disabled when running a romhack to prevent conflicts with remapped levels and modified game logic. These options are greyed out in the Enhancements menu.

## For Romhack Creators

If you're building a romhack with BB and want it to work on Lighthouse:

- All BB General Settings are supported. Configure freely.
- Assets (models, textures, music, dialogs, sprites, map setups) are extracted normally from the asset table.
- Actors from any level overlay can be used on any map — the port registers all overlay actors globally for romhacks.
- Test with the vanilla US v1.0 ROM as your base.

See [BACKPACKCODE.md](BACKPACKCODE.md) for the technical details of how extraction works.
