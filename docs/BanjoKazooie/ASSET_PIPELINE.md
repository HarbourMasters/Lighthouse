# Asset Pipeline

This document explains how Banjo-Kazooie asset data flows from ROM to runtime, the formats used at each stage, and what makes BK's pipeline different from other HarbourMasters ports.

## Asset Symbol Map

All assets are mapped by numeric ID in `assets.yml`. The decomp references assets by ID; ResourceHelpers resolves IDs to o2r paths via a manifest blob (`aBKAssetTable`) loaded at startup.

Key ID ranges:
- Animation assets: IDs up to 0x2C9 (713)
- Model assets: ID + 0x2D1 (721)
- Sprite assets: ID + 0x572

Loaded resources are retained in `sResourceRefCache` to prevent LUS from evicting them while the decomp still holds raw pointers.

---

## Bitfield Repacking

This is the most pervasive BK-specific concern. The N64 decomp declares C bitfield structs in big-endian order (first member = MSB). On little-endian platforms, the compiler places the first declared member at the LSB. Every importer that produces data read through bitfield structs must repack fields into LE order.

Example — NodeProp `radius`/`bit6`/`bit0`:
```
BE declaration: u16 radius:9; bit6:6; bit0:1;
  -> BE layout: [radius 15:7] [bit6 6:1] [bit0 0]

LE repacking: radius at bits 0-8, bit6 at bits 9-14, bit0 at bit 15
  -> (radius & 0x1FF) | ((bit6 & 0x3F) << 9) | ((bit0 & 0x01) << 15)
```

This pattern applies to NodeProp, Prop, animation file elements, and sprite metadata.

---

## Model Format

Models are the most complex asset. The importer produces a single contiguous blob matching `BKModelBin` layout:

```
[Header 0x30 bytes — section offsets]
[Geo layout]
[Texture metadata + contiguous pixel data]
[Animation list]
[Collision grid + triangles]
[Unk14 list]
[Unk20 list]
[Effects / mesh lists]
[Unk28 list]
[Animated texture list]
[Vertex list]
[Display list]
```

All sections are 8-byte aligned. The header stores byte offsets to each section.

### Display List Widening

N64 display list commands are 8 bytes (two u32 words). The port widens them to 16 bytes (two u64 words) for 64-bit pointer support. Torch embeds raw N64 command words in the o2r; the importer widens them in-place during blob construction.

Offset scaling rules:
- **G_DL** (opcode 0x06): byte offset × `sizeof(Gfx) / 8` (2× on 64-bit)
- **G_VTX** (opcode 0x04): `(old_offset / 16) * sizeof(Vtx)` — N64 Vtx is 16 bytes, PC Vtx with `GBI_FLOATS` is 24 bytes

### Segmented Address Tagging

Commands that carry segmented addresses (G_MTX, G_MOVEMEM, G_VTX, G_DL, G_SETTIMG, G_SETZIMG, G_SETCIMG) have their w1 tagged with `w1 |= 1`. At runtime, `SegAddr()` tests `w1 & 1` — if set, it extracts the segment number and offset and resolves through the segment table. Literal values in other commands must not be tagged.

### Why BK Embeds Raw DLs (vs. Other Ports)

Other HarbourMasters ports (SM64, SF64, MK64) export display lists as separate sub-resources that the importer loads individually. BK's geo layout system references display lists by array index (`list[cmd->unk8]`), and models can have shared texture regions with inter-texture gaps that DL commands reference directly. Embedding the raw command stream and widening in-place preserves these index relationships without restructuring the geo layout.

### Texture Data

Torch captures the entire decompressed texture data region as a **raw contiguous blob**, written inline in the model's binary data. This preserves data between listed textures that display list commands may reference (unlisted padding, shared pixel regions). The importer uses this blob directly.

### Geo Layout Offsets

The geo layout is a hierarchical scene graph. Sibling/child references are byte offsets within the layout buffer. Torch writes each command at its original ROM byte position so these offsets remain valid without remapping.

---

## Map Format

Maps use a two-stage approach unique to BK.

### Stage 1: Torch Binary

Torch reads cube data from ROM and exports:
- Cube grid bounds (min/max position)
- Per-cube: header (x/y/z grid coords, prop counts) + NodeProp array + Prop array
- Camera node data, lighting data

NodeProp fields are exported as individual typed values (s16 position, u16 radius, u8 bit6, etc.) — NOT as packed bitfields.

### Stage 2: Importer Chunked Serialization

The importer does NOT pass data directly to the decomp. Instead, it reconstructs a **chunked byte stream** that mimics the ROM's format, because the decomp's runtime parsers (`cubeList_fromFile` in `actor_cubebounds.c` and `code7AF80_initCubeFromFile` in `actor_cubepropsystem.c`) read data sequentially using specific chunk markers:

| Marker | Meaning | Parsed by |
|--------|---------|-----------|
| 0x00 | Terminator | `actor_cubebounds.c` |
| 0x01 | Section boundary | `actor_cubebounds.c` |
| 0x02 | Camera node type | `actor_cubebounds.c` |
| 0x03 | Block start (cube, camera, light) | `actor_cubebounds.c` |
| 0x04 | Lights section start | `actor_cubebounds.c` |
| 0x06 | OtherNode count follows | `actor_cubepropsystem.c` |
| 0x07 | OtherNode data follows | `actor_cubepropsystem.c` |
| 0x08 | Prop count follows | `actor_cubepropsystem.c` |
| 0x09 | Prop data follows | `actor_cubepropsystem.c` |
| 0x0A | NodeProp count follows | `actor_cubepropsystem.c` |
| 0x0B | NodeProp data follows | `actor_cubepropsystem.c` |

`WriteNodeProp()` repacks individual fields into LE bitfield layout (see Bitfield Repacking above). Props are byte-swapped from BE to native endian.

### Why Two Stages (vs. Other Ports)

SM64/SF64/MK64 ports typically have Torch write final native structs that the importer passes through directly. BK's runtime code uses streaming chunk parsers inherited from the original ROM loader — the decomp expects to read marker bytes and parse sequentially. Rewriting the decomp's chunk parser would touch dozens of functions across the codebase, so the importer preserves the protocol with native-endian values.

### Map Data Contents

- **Cubes**: Spatial grid cells (signed 5-bit x/y/z coordinates in the runtime struct, extracted as unsigned 0-31 by Torch), each containing NodeProp and Prop arrays
- **NodeProps** (20 bytes): Position, radius, yaw, scale, and type-specific flag bitfields. Used for spawn triggers, cameras, spline paths (`spline_pathfollow.c`), and events.
- **Props** (12 bytes serialized, 16 bytes on 64-bit runtime): Union of ActorProp, SpriteProp, ModelProp. Discriminated by flag bits. Contains position and asset references.
- **Camera Nodes**: Types 0-4 with position, speed, rotation, distance data (see ARCHITECTURE.md)
- **Lights**: Position, inner/outer fade radii, RGB color

---

## Sprite Format

Sprites have a complex per-frame sub-resource structure.

### Header

Torch exports the sprite header fields individually:
- `formatCode` — texture format (CI4, CI8, RGBA16, etc.)
- `unk4`/`unk6` — undocumented display parameters
- `unk8`/`unkA` — display width/height (used for vertex scale in billboard rendering)
- `unkC` fields — animation speed, type, direction, flip (4 separate u8s from the original bitfield)

### Frames

Each frame contains:
- Header data (9 × s16 from Torch, 10 × s16 in runtime `BKSpriteFrame` which adds `chunkCnt`): x, y, w, h, and chunk layout metadata
- Chunk position arrays (x, y pairs)
- Per-frame palette data loaded as `_frameIdx_TLUT` sub-resources (for CI4/CI8 formats)
- Chunk texture data loaded as `_frameIdx_chunkIdx` sub-resources

The importer assembles these sub-resources into `BKSpriteFrame` structs with inline texture block data. The final `BKSprite` is NOT a simple cast-to-struct blob — it's a runtime-populated structure with per-frame pointers.

---

## Animation Format

Torch exports animation data as:
- Header: start frame, end frame, element count
- Per element: bone index, transform type (rotation/scale/translation component), keyframe count
- Per keyframe: spline flags bitfield (u16) + interpolation value (separate s16, divided by 64.0 at runtime for float recovery)

The importer repacks the per-element and per-keyframe bitfields into LE order:
```
Element: u16 = (transformType << 12) | boneIndex
Keyframe: u16 = (value << 2) | (flag2 << 1) | flag1
```

The resulting blob is cast to `AnimationFile` by the decomp and decoded inline during skeletal animation playback.

---

## Dialog / Quiz Format

Dialog and quiz assets use a chunked command format:
- Command bytes interleaved with text data
- Bottom text box and top text box stored as separate command sequences
- Quiz questions include answer metadata (correct answer index, category)

The importer reconstructs the ROM-format header (version markers and offset tables) and converts Torch's wider types (u32 lengths) back to the compact ROM format (u8 lengths), since the decomp's dialog parser expects the exact command byte protocol.
