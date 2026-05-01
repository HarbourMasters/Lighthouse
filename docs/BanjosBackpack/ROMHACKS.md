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

## Unsupported: Romhacks with Custom MIPS Code Injection

Romhacks built with tools that go beyond BB's data-only patching — i.e., that inject custom MIPS code into the ROM — are **not supported** and will appear partially broken on Lighthouse. Telltale symptoms include scripted scenes that play on N64 but do nothing on the port (custom title pages, story sequences, novel cutscenes), and per-frame logic that reacts to input on certain maps but not others.

### Why this happens

BB-style romhacks only modify named data fields (scene remaps, warp destinations, music, item caps, etc.) which Torch extracts into `aGameConfig` and the port honors at runtime. Lighthouse can faithfully reproduce that behavior because the underlying engine code path is the same as vanilla.

A custom-code romhack additionally relocates a chunk of MIPS code into a high-RAM region (e.g., `0x80780000+` on N64), then patches one or more `jal` instructions inside vanilla functions to dispatch into that custom code each frame. The injected code typically:

- Maintains its own per-frame state machine (page index, input edge detection, etc.)
- Calls vanilla engine functions like `gcdialog_showDialog` directly
- Sometimes hooks into draw/update functions to render custom UI

Lighthouse runs the decompiled C source — not the original MIPS — so the patched `jal` is gone, the custom code is never loaded, and the romhack's scripted behavior is silently absent.

### Diagnosing it

A confirmed example is *How the Gruntch Stole Christmas*. On N64, stick-right on the title page advances story pages; on the port, stick-right does nothing. Live RAM inspection of the running N64 ROM showed:

- A vanilla function (`gcdialog_showDialog`) being invoked with arguments that no decompiled caller produces.
- A 128 KB code blob loaded at `0x80780000` containing the page-flip handler and a thin wrapper that translates flag bits into the engine's dialog API.
- A `jal 0x807813A0` patched into the vanilla per-frame draw function, dispatching into that blob every frame.

Equivalent symptoms in other romhacks generally point to the same pattern.

#### Reconstructed wrapper (gruntch example)

Disassembling the captured blob gave the wrapper at `0x807801E0` that the gruntch hack uses to drive dialog calls. It takes a text id and a 4-bit flag word, decodes the flags into a `gcdialog_showDialog` arg1, optionally toggles a file-progress flag across the call, and forwards a position pointer:

```c
// Reconstructed from MIPS at 0x807801E0
//
// flag_bits decoding:
//   bit 0 (0x1)  pass `pos` through (0 forces NULL)
//   bit 1 (0x2)  manual advance       -> arg1 |= 0x80,  and arg1 |= b0<<3
//   bit 2 (0x4)  toggle FILEPROG_1E for the duration of the call
//   bit 3 (0x8)  request balookat lock -> arg1 |= 0x2
//
// Resulting gcdialog_showDialog arg1:
//   b1=0,b3=0:  arg1 = b0                       (0 or 1)
//   b1=0,b3=1:  arg1 = b0 | 0x2                 (2 or 3)
//   b1=1,b3=0:  arg1 = 0x80 | (b0<<3)           (0x80 or 0x88)
//   b1=1,b3=1:  arg1 = 0x80 | (b0<<3) | 0x2     (0x82 or 0x8A)
//
// The story-page handler at 0x8078E3A4 calls this with flag_bits=0xB
// (b0=1,b1=1,b3=1, no b2), giving arg1=0x8A — which is exactly the
// fingerprint observed via UDP RAM scan when the N64 fires page 0xA1D.

void custom_dialog_wrapper(s32 text_id, s32 flag_bits, f32 *pos) {
    s32 b0 = flag_bits & 0x1;
    s32 b1 = flag_bits & 0x2;
    s32 b2 = flag_bits & 0x4;
    s32 b3 = flag_bits & 0x8;

    s32 arg1 = b0;
    if (b1) arg1 = (arg1 << 3) | 0x80;
    if (b3) arg1 |= 0x2;

    // "& -b0" idiom: pos passes through if b0==1, becomes NULL if b0==0.
    pos = (f32 *)((uintptr_t)pos & (uintptr_t)(-b0));

    if (b2) {
        // Side-channel: temporarily set FILEPROG_1E_LAIR_GRATE_TO_BGS_PUZZLE_OPEN
        // across the call, then restore. The romhack uses this otherwise-unused
        // flag as a transient gate visible to dialog callbacks.
        s32 old = fileProgressFlag_getAndSet(FILEPROG_1E_LAIR_GRATE_TO_BGS_PUZZLE_OPEN, 1);
        gcdialog_showDialog(text_id, arg1, pos, NULL, NULL, NULL);
        fileProgressFlag_set(FILEPROG_1E_LAIR_GRATE_TO_BGS_PUZZLE_OPEN, old);
    } else {
        gcdialog_showDialog(text_id, arg1, pos, NULL, NULL, NULL);
    }
}
```

The story-page handler at `0x8078E3A4` (~3.2 KB) drives a state machine over multiple dialog ids (e.g. `0xA13`, `0xA1D`, …) by calling this wrapper each time the player advances a page. None of that state machine exists in the decompiled source — it's entirely inside the injected blob.

#### Wrapper call sites in the gruntch blob

Disassembling the blob found 22 invocations of the wrapper, all reusing a small set of vanilla asset IDs that the romhack has retextured/restringed. Sorted by text id:

| call site (RAM) | text_id | flag_bits | derived arg1 | pos | vanilla asset (retextured) |
|---|---|---|---|---|---|
| 0x8078ea44 | 0xA12 | 0xB | 0x8A | NULL | Black Snippet Spawned |
| 0x8078eb0c | 0xA13 | 0x9 | 0x82 | yes | Black Snippet Defeated |
| 0x8078ecbc | 0xA19 | 0xB | 0x8A | NULL | Treasure Hunt 2nd Step |
| 0x8078e9d0 | 0xA1B | 0xA | 0x82 | NULL | Snacker Spawned 1 |
| 0x8078e7c0 | 0xA1C | 0xA | 0x82 | NULL | Snacker Spawned 2 |
| **0x8078eb70** | **0xA1D** | **0xB** | **0x8A** | **NULL** | **Snacker Spawned 3 (page 1)** |
| 0x8078ef14 | 0xA1E | 0xB | 0x8A | NULL | Snacker Spawned 4 (page 2) |
| 0x8078e844 | 0xA29 | 0xB | 0x8A | NULL | Snacker Bite |
| 0x8078eeb4 | 0xA70 | 0xB | 0x8A | NULL | Rubee Helped |
| 0x8078eed4 | 0xA72 | 0xB | 0x8A | NULL | Trunker Helped |
| 0x8078eef4 | 0xA73 | 0xB | 0x8A | NULL | Gobi Meet |
| 0x8078ef34 | 0xA74 | 0xB | 0x8A | NULL | Gobi Helped |
| 0x8078ee94 | 0xA75 | 0xB | 0x8A | NULL | Gobi2 Meet |
| 0x8078e674 | 0xA76 | 0xB | 0x8A | NULL | Gobi2 Done |
| 0x8078e8b4 | 0xA77 | 0xB | 0x8A | NULL | Gobi3 Done |
| 0x8078fcac | 0xA7B | (reg) | 0x8A | NULL | Jinxy Meet (id loaded from register) |
| 0x8078e5c8 | 0xA7C | 0xB | 0x8A | NULL | Jinxy One Egg |
| 0x8078ee0c | 0xA7D | 0x9 | 0x82 | yes | Jinxy Helped |
| 0x807826b0 | 0xA7E | 0x8 | 0x80 | NULL | Sand Eels Meet |
| 0x8078177c | 0xA81 | 0x8 | 0x80 | NULL | Sandybutt Enter |
| 0x8078e740 | 0xA82 | 0xB | 0x8A | yes | Sandybutt Start Maze |
| 0x8078e934 | 0xA83 | 0xB | 0x8A | NULL | Sandybutt Done |

The captured fingerprint `gcdialog_showDialog(0xA1D, 0x8A, NULL, NULL, NULL, NULL)` corresponds exactly to the call site at `0x8078eb70`, which is the first stick-right page in the title-screen story handler.

Note that the romhack never uses the `b2` (FILEPROG_1E side-channel) path in any of these call sites — only `0x8`, `0x9`, `0xA`, `0xB`. The `b2` path exists in the wrapper but is dead code in this build.

### What we won't do

- Run an interpreter for arbitrary MIPS code blobs. Out of scope and would be fragile per romhack.
- Detect specific romhacks by hash and bake in custom Lighthouse-side reimplementations of their scripted scenes.

### What romhack authors can do

If your hack relies on custom MIPS code, port the equivalent logic into Lighthouse's enhancement/event hook system and gate it behind your romhack's name. This requires a Lighthouse source contribution — there's no data-only path from a custom-code romhack to a working port build.
