# Making a Model Mod

A custom model replaces one of the game's models with your own. This guide walks through what a model is made of, how the game finds yours, and how to build one in Blender with Fast64. Animations have [their own guide](CUSTOM%20ANIMATIONS.md).

---

## 1. The model container

A Banjo-Kazooie model is not one file but a family of sibling resources sharing a base path. Every model Torch extracts has this shape (`assets/model/ASSET_<id>_<label>` plus suffixes), and a custom model uses the same one:

| Entry | Type | Contents |
|---|---|---|
| `<name>` | `BKMO` | the model header: geo type, tri/vert counts, presence flags, bounding box + cull sphere, the display list (raw N64 command words), and the bone table |
| `<name>_VTX` | `OVTX` | vertex count + N64 `Vtx` records (16 bytes each: position, flag, UV, color/normal) |
| `<name>_GEO` | `OBLB` | the geometry layout: a small command tree (`BONE`, `LOADDL`, ...) that binds ranges of the display list to bones |
| `<name>_tex_<i>` | `OTEX` | one texture: format, width, height, raw pixels in native N64 encoding |

Each one carries the standard 64-byte header every `bk.o2r` entry has, so the engine loads a custom model exactly as it loads an extracted one. For the finer details, read the code that loads them: [`src/port/Resource/Importers/ModelFactory.cpp`](../../src/port/Resource/Importers/ModelFactory.cpp) with the structs in [`include/core2/model.h`](../../include/core2/model.h), and Torch's `docs/otr_format/` for the vertex/texture/blob bodies.

Three things to check when you produce one:

- **Images and palettes bind by path.** A `G_SETTIMG` in the display list whose address is `0xFF000000 | i` resolves at load time to `<model path>_tex_<i>`. That is what makes each image replaceable on its own, HD included (see the [texture pack guide](TEXTURE%20PACKS.md)). A CI texture's palette comes from the `<model path>_tex_<i>_TLUT` Torch writes beside it, so repainting that recolors the texture, with [one exception](TEXTURE%20PACKS.md#ci-textures-and-their-tluts).
- **The geo layout references display-list command *indices*.** Each `BONE` node names a bone's animation matrix and a `LOADDL` child pointing at where that bone's geometry starts in the command array, so the order of the commands matters.
- **The bone table is mandatory for anything animated.** The header ends with an animation scale and a flat bone list (rest position, id, parent). The animated-actor path dereferences the skeleton without a null check - a boneless model in an animated slot crashes the game. Bones are what [animation YAML](CUSTOM%20ANIMATIONS.md) addresses by id.

BK's skinning is **rigid**: there are no vertex weights, and each *vertex* follows exactly one bone ([`rendernormal.c:31`](../../src/core2/model/rendernormal.c#L31)). A bone's matrix moves all of its vertices together from their rest positions. Where two bones meet, the seam closes because a triangle's corners follow different bones, so put joints on natural rings in the geometry where the stretch hides.

---

## 2. Wiring a model into the game

Put the whole model family in your mod's `.o2r` under `alt/`, at the original model's path (`alt/assets/model/ASSET_<id>_<label>` and its siblings). It then replaces the original while **Alternate Assets** is on, the same as a texture pack. Every vanilla animation that plays on that actor still addresses the *original* skeleton's bone ids, so your bone table has to answer to them. Extend the original skeleton rather than inventing one.

Swap Banjo for the blue Jinjo: copy the Jinjo's whole family out of `bk.o2r` (`assets/model/ASSET_3C0_JINJO_BLUE` plus its `_GEO`, `_VTX`, and four `_tex_*`/`_TLUT` pairs - eleven entries), rename each to the matching `alt/assets/model/ASSET_34E_BANJOKAZOOIE_HIGH_POLY` name, `torch pack` the folder into an `.o2r`, drop it in `mods/`. It *works*: a blue Jinjo stands in Banjo's place.

Test it indoors, in Banjo's house or the lair. The world maps and several cutscenes draw the low-poly `ASSET_34D_BANJOKAZOOIE_LOW_POLY` instead, unless **Disable LOD** is on ([`playermodel.c`](../../src/core2/ba/playermodel.c)), so swap `34D` too for Banjo to change everywhere.

Then walk around: the Jinjo's legs hold their T-pose while it slides along, and there's no Kazooie at all. The walk animation addresses *Banjo's* bone ids, and the Jinjo numbers its own limbs differently. No crash - unaddressed bones hold their rest pose.

Loose files become an archive with Torch's packer - it zips a staging folder verbatim, so the folder's layout is the archive's layout:

```
torch pack <staging-folder> <name>.o2r o2r
```

---

## 3. Building a model

[Fast64](https://github.com/HarbourMasters/fast64)'s HarbourMasters fork carries a **BK64** game mode that reads and writes the container from [#1](#1-the-model-container).

Install the fork (zip the repo, then Blender's *Preferences -> Add-Ons -> Install*) and set the game to BK64 in the Fast64 tab. The model and animation tools appear in the 3D view sidebar under a BK64 tab. The plugin imports a model or skeleton from a resource family or a `.bin` - mesh, UVs, vertex colors, textures, materials, collision data and shapes, mesh lists, and the armature with its original bone ids - and exports the same back to either format. Animations go both ways too, matched by bone id, one action at a time or every action on the rig at once.

Import the original model first. The bone table arrives with its ids intact, and the animation scale comes with it, so the vanilla animations find the right limbs and move them the right distance. Fit your mesh to that armature, set Resource Path to the vanilla model's own path, export, and pack the export with `torch pack` as above, under `alt/`.

The plugin's [own README](https://github.com/HarbourMasters/fast64/blob/main/fast64_internal/hm64/bk64/README.md) is the reference for the Blender side - materials and draw layers, the two rigging methods, geo nodes, collision, animated textures, and what doesn't survive a round trip.

**Torch imports a model's geo layout and textures, not its mesh.** The `torch modding export` flow the [animation](CUSTOM%20ANIMATIONS.md) and [dialog](CUSTOM%20DIALOG.md) guides start from writes each model's `_GEO.yaml` and its textures as PNGs. The mesh comes through Blender, and Torch's only job there is `torch pack`.
