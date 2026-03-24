# Banjo's Backpack — Technical Reference

Technical documentation for how Lighthouse extracts and imports BB romhack configuration.

## Data Sources

BB modifies three data segments in the ROM. Torch's `ConfigFactory.cpp` extracts from all three and writes a binary `aGameConfig` blob (BKCF format) into the o2r.

| Segment | Description | Compressed? | How We Find It |
|---------|-------------|-------------|----------------|
| **F37F90** | Core2 overlay (~903 KB) | BKZIP | EOR boot code pointer pairs |
| **F9CAE0** | Level config data (~92 KB) | BKZIP | Scan after F37F90 for BKZIP with ~92 KB expected size |
| **FCF698** | Lair overlay objects (9,888 bytes) | No | Fixed ROM offset 0x3F5CDB0 in globalized blob |

### Finding F37F90 (Patched Overlay)

BB relocates the core2 overlay to extended ROM space. The new address is encoded in EOR (End Of Rom) pointer pairs — split `LUI`/`ADDIU` MIPS instructions in the boot code. We try multiple pairs until we find one that decompresses to ~900 KB via BKZIP (magic `0x1172`).

EOR pairs checked: `{0x107E,0x1086}`, `{0x2872,0x287A}`, `{0x287E,0x2886}`, `{0x288A,0x2892}`, `{0x2896,0x289A}`, `{0x28EA,0x28F2}`, `{0x28F6,0x28FE}`, `{0x2902,0x290A}`, `{0x290E,0x2912}`.

### Finding F9CAE0 (Relocated Data Segment)

BB writes the modified F9CAE0 immediately after F37F90 in the extended ROM area. The copy at the original ROM offset `0xF9CAE0` is stale (vanilla). We scan forward from the F37F90 address looking for a BKZIP header with 80,000–100,000 expected bytes. Both the relocated (modified) and original (vanilla) copies are decompressed and diffed.

### FCF698 (Lair Overlay Objects)

The Globalizer concatenates all 13 level overlays into a blob at ROM `0x3F00000`. The Grunty's Lair Objects data ends up at a fixed offset within this blob. BB hardcodes this at ROM address `0x3F5CDB0` (9,888 bytes, uncompressed). We read it directly and diff against known vanilla defaults.

---

## F37F90 Code Constants

Byte offsets within the decompressed core2 overlay. Each is either a single byte or a BE u16 (MIPS immediate field). Compared against hardcoded vanilla defaults.

### Start Level

| Offset | Key | Vanilla | Format |
|--------|-----|---------|--------|
| 254331 | `new_game_map` | 0x85 | byte |
| 624378 | `start_level_1` | 0x01 | byte |
| 625582 | `start_level_2` | 0x69 | byte |

### Abilities

| Offset | Key | Vanilla | Format |
|--------|-----|---------|--------|
| 59470 | `know_all_moves` | 0xC3A0 | u16 BE |

### Mumbo Transformation Costs

| Offset | Key | Vanilla | Format |
|--------|-----|---------|--------|
| 305126 | `mumbo_cost_termite` | 5 | u16 BE |
| 305134 | `mumbo_cost_croc` | 10 | u16 BE |
| 305142 | `mumbo_cost_walrus` | 15 | u16 BE |
| 305150 | `mumbo_cost_pumpkin` | 20 | u16 BE |
| 305158 | `mumbo_cost_bee` | 25 | u16 BE |

### Item Max Capacities

| Offset | Key | Vanilla | Format |
|--------|-----|---------|--------|
| 782878 | `eggs_normal_max` | 100 | u16 BE |
| 782870 | `eggs_cheato_max` | 200 | u16 BE |
| 782910 | `red_feathers_normal_max` | 50 | u16 BE |
| 782902 | `red_feathers_cheato_max` | 100 | u16 BE |
| 782938 | `gold_feathers_normal_max` | 10 | u16 BE |
| 782934 | `gold_feathers_cheato_max` | 20 | u16 BE |
| 19162 | `notes_max` | 100 | u16 BE |

Notes max is written to 4 redundant locations (19162, 568358, 783338, 786094). We check only the first.

### World/Honeycomb Settings

| Offset | Key | Vanilla | Format |
|--------|-----|---------|--------|
| 568414 | `jiggies_per_world` | 10 | u16 BE |
| 568490 | `honeycombs_per_world` | 2 | u16 BE |
| 568494 | `extra_hc_start` | 6 | u16 BE |
| 568479 | `special_level` | 0x0B (SM) | byte |
| 570539 | `hide_jiggies_level` | 0x0B (SM) | byte |
| 571219 | `hide_collectibles_level` | 0x06 (Lair) | byte |

### Special Warp Destinations

| Offset | Key | Vanilla | Format |
|--------|-----|---------|--------|
| 0x986FA | `warp_exit_banjos_house` | 0x0112 | u16 BE |
| 0x98BAE | `warp_enter_lair` | 0x6912 | u16 BE |

Values encode `map_id << 8 | exit_id`.

---

## F9CAE0 Level Configuration Tables

Diffed field-by-field between the relocated (modified) and original (vanilla) decompressed copies.

### Scene-to-Level Association (offset 0x8284)

8-byte entries. BB reads scene_id at +4 (u16 BE) and writes level_id at +6 (byte). Our extraction reads from offset 0x8288 (= 0x8284 + 4) where scene_id is at +0 and level_id at +2 within our view. Determines which world a map belongs to (`D_8036B810`).

### Return-to-Lair Table (offset 0x8FD0)

11 entries of 4 bytes each. Map ID at +0 (u16 BE), exit ID at +2 (u16 BE). Controls where `gcpausemenu_returnToLair()` sends the player when exiting a world (`D_8036C560`).

### Music Assignments (offset 0xA8F0)

8-byte entries. Map ID at +0 (u16 BE), primary track at +2 (u16 BE), secondary track at +4 (u16 BE).

### Skybox Assignments (offset 0x87B0)

40-byte entries (`MapSkyInfo` struct). Up to 3 skybox layers per scene:

```
+0:  s16 scene_id
+4:  s16 model1,  +8: f32 scale1,  +12: f32 rotation1
+16: s16 model2, +20: f32 scale2, +24: f32 rotation2
+28: s16 model3, +32: f32 scale3, +36: f32 rotation3
```

BB writes scale values as the upper 16 bits of the f32 (e.g. `0x3F80` = 1.0f). We read the full 4-byte f32.

### Scene Definitions (offset 0x7650)

24-byte entries (`MapModelDescription` struct). Scene ID at +0 (u16 BE), OPA model at +2 (u16 BE), XLU model at +4 (u16 BE), min bounds at +6 (s16[3] BE), max bounds at +12 (s16[3] BE), padding at +18 (2 bytes), scale at +20 (f32 BE). Bounds are offsets added to the model's vertex bounds for collision spatial grid sizing.

### Level Names (offset 86068–86302)

Null-terminated ASCII strings for 13 level names. Used in `D_8036C58C` (pause menu display). Diffed against vanilla strings.

---

## FCF698 Lair Overlay Data

Read from ROM `0x3F5CDB0`, 9,888 bytes uncompressed. Diffed against known vanilla defaults.

### Note Door Thresholds (offset 1996)

12 u16 BE values — notes required to open each door in Grunty's Lair (`D_8039347C`).

### Jiggy Puzzle Costs (offset 6984)

11 entries, stride 4. Cost byte at +0. Jiggies required for each puzzle door (`D_803947F8`).

---

## BKCF Binary Blob Format

All multi-byte values are native host endian (LE on x64).

```
Header:
  u32 magic = 'BKCF' (0x46434B42)
  u16 version = 1
  u16 section_count
  u8  name_len
  char name[name_len]    (o2r name, e.g. "bk-jot")

Per Section:
  u16 section_type
  u16 entry_count
  [entries...]
```

### Section Types

| ID | Name | Entry Format | Bytes/Entry |
|----|------|-------------|-------------|
| 1 | CODE_CONSTANTS | `{u16 key_id, u16 value}` | 4 |
| 2 | SCENE_REMAP | `{u16 map_id, u16 level_id}` | 4 |
| 3 | RETURN_TO_LAIR | `{u8 index, u8 pad, u16 map_id, u16 exit_id}` | 6 |
| 4 | MUSIC | `{u16 map_id, u16 track1, u16 track2}` | 6 |
| 5 | SKYBOX | `{u16 scene_id, 3x{s16 model, u32 scale, u32 rot}}` | 32 |
| 6 | SCENE_DEF | `{u16 scene_id, s16 opa, s16 xlu, s16 min[3], s16 max[3], u32 scale}` | 22 |
| 7 | NOTE_DOORS | `{u8 door_index, u8 pad, u16 threshold}` | 4 |
| 8 | JIGGY_PUZZLES | `{u8 puzzle_index, u8 cost}` | 2 |
| 9 | LEVEL_NAMES | `{u8 level_index, u8 len, char name[len]}` | variable |
| 10 | WARP_DESTINATIONS | `{u16 warp_index, u16 dest}` | 4 |

Empty sections are omitted. Only values that differ from vanilla are included.

### Warp Destination Extraction

BB's Warps tab patches MIPS instructions inside warp functions in the F37F90 code overlay to change where each warp leads. Each warp function follows a pattern (from BB source `GeneralSettings.cs`):

```
27BDFFE8  ADDIU $sp, $sp, -0x18    (prologue)
AFBF0014  SW $ra, 0x14($sp)
AFA?001?  SW $a0/$a1, 0x18/0x1C($sp)
...
0C0C????  JAL func_8031CC8C (or similar warp target)
2405XXYY  ADDIU $a1, $zero, dest   (branch delay slot — THIS is the dest)
8FBF0014  LW $ra, 0x14($sp)        (epilogue)
27BD0018  ADDIU $sp, $sp, 0x18
03E00008  JR $ra
00000000  NOP
```

BB uses a regex to find and patch the `ADDIU $a1, $zero, dest` instruction in the **branch delay slot after the JAL**. The dest encodes `scene_id << 8 | entry_id`.

**Extraction process:**
1. Read the warp function pointer table from F9CAE0 at offset 0xC3F0 (558 entries, 4-byte N64 addresses)
2. Convert each N64 address to a byte offset in F37F90: `offset = addr - 0x80286D10`
3. Scan 200 bytes at that offset for a JAL followed by `ADDIU $a1, $zero, imm16` in the delay slot
4. Compare the dest between vanilla and modified F37F90 overlays
5. Emit diffs as WARP_DESTINATIONS entries

At runtime, `nodeupdate.c` intercepts the warp dispatch. Before calling `sWarpFunctions[idx]`, it checks `port_getRomhackWarpDest(idx)`. If an override exists, it calls `func_8031CC8C(arg0, dest)` directly with the new destination, bypassing the original function.

---

## Runtime (Port Side)

`LoadGameConfig()` in `GameConfigFactory.cpp` reads `assets/aGameConfig` from the o2r archive, parses the BKCF sections, and caches all values. Decomp code calls `port_getRomhack*()` accessors (declared in `GameConfig.h`) which return the override value or `-1` for vanilla default.

On vanilla ROMs (no `aGameConfig`), `port_isRomhack()` returns false and all accessors short-circuit to their default return without any map lookups.

### Warp Interception

`nodeupdate.c` `func_80334448()` is the warp dispatch point. Before calling `sWarpFunctions[idx](arg0, arg1)`, it checks `port_getRomhackWarpDest(idx)`. If an override exists, it calls `func_8031CC8C(arg0, dest)` directly with the remapped destination, bypassing the original warp function.

Note: `start_level_1` and `start_level_2` in BB's Start Level tab share offsets with `WARP_EXIT_BANJOS_HOUSE` and `WARP_ENTER_LAIR` respectively. BB writes a single byte (the map ID) which becomes the high byte of the u16 warp destination. These are captured as WARP_DESTINATIONS entries, not separate config values.

### Actor Registry

BB's Globalizer makes all 13 level overlays resident simultaneously. On the port, all overlays are statically linked but their actor registration functions only run for the current level's overlay. When `port_isRomhack()` is true, `spawn_queue.c` calls every overlay's actor registration function so actors from any level can spawn on any map.

### Disabled Enhancements

Some port enhancements are incompatible with romhacks and are automatically disabled when `port_isRomhack()` returns true. The UI greys out these options with a `DISABLE_FOR_ROMHACK` flag. Currently disabled:

- **Return to Lair** — romhacks may reassign lair maps
