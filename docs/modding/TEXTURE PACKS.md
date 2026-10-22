# Making a Texture Pack

A texture pack is an `.o2r` archive that replaces any of the game's textures with your own artwork. It can redraw them at the same resolution or upscale them (HD). Players drop it into their `mods/` folder, where it applies on top of the base game and can be switched off in game.

Packs are built end to end with **[Retro](https://github.com/HarbourMasters/retro)**, HarbourMasters' OTR/O2R generation tool. Retro extracts every texture to PNG and packs the ones you change. This guide recolors the blue eggs green.

---

## 1. Extract the game's textures

Open Retro and go **Create OTR / O2R -> Replace Textures**. It asks whether you already have a texture replacement folder - answer **No** the first time, then:

1. Select your generated `bk.o2r` (the one your Lighthouse install plays from, so the entry names match exactly).
2. Pick an output folder.

Retro writes a `bk/` folder there containing every texture as a PNG, at the same relative path it has inside the archive. Next to the textures is a **`manifest.json`** recording each texture's hash, format, and size.

Two rules about this folder:

- **Don't touch `manifest.json`.** It is how Retro knows a texture's native format and whether you changed it. Packing refuses to run without it.
- **Don't move or rename the PNGs.** The relative path *is* the archive entry name; a moved file no longer matches the manifest and is silently dropped at pack time.

Re-running the extraction into the same folder deletes and recreates it, so keep your edits elsewhere until they're final, or extract once and back the folder up.

Retro also writes `textures/font/sGfxPrintFontData.png` into the folder for every game. Lighthouse never loads it, so leave it alone.

Every base names its assets the same way, so a pack extracted from one region's `bk.o2r` works on the others.

### How the files are named

Entry names follow the base game's own layout - `assets/<category>/ASSET_<id>_<label>` plus a suffix for each image the asset owns:

| Pattern | What it is | Example |
|---|---|---|
| `..._tex_<i>` | the i-th texture of a model or level | `assets/model/ASSET_491_NOTE_DOOR_tex_1` |
| `..._<frame>_<chunk>` | one chunk of a sprite frame | `assets/sprite/ASSET_36D_BLUE_EGG_0_0` |
| `..._TLUT` | the palette of a CI4/CI8 texture | `assets/sprite/ASSET_36D_BLUE_EGG_0_TLUT` |

The `ASSET_<id>` numbers are the game's own asset ids, so anything you find in the asset table or other docs maps straight onto a folder path.

---

## 2. Edit the textures

Edit any PNG in place with whatever editor you like. At pack time Retro compares every file's hash against the manifest and **stages only the ones you changed**. You never trim the folder down by hand, and unedited files cost nothing.

RGBA, I and IA textures come out as ordinary images, so you can edit those directly.

### CI textures and their TLUTs

Nearly half the game's textures are **palettized** (CI4/CI8): the texture stores per-pixel *indices*, and a separate 16- or 256-entry palette - the `_TLUT` sibling - supplies the colors. Retro extracts the two halves as two PNGs and does not join them back up. The texture comes out as an indexed PNG with a neutral grayscale ramp for a palette, and the `_TLUT` PNG is a tiny strip holding the actual colors. So a CI texture opens **black and white**: raw palette slots drawn without their colors.

**At pack time Retro reads back only the indices**, which slot each pixel uses. In-game the colors still come from the TLUT. Three workflows cover every case:

- **Recoloring: edit the `_TLUT` strip and nothing else.** This is our egg example: the blue egg is sprite `ASSET_36D_BLUE_EGG`, and repainting the blue entries in its `_TLUT` PNGs green recolors the egg everywhere that sprite is drawn. Models and levels work the same way, with one exception below.
- **Redrawing in place: give the PNG its real palette first.** The TLUT strip *is* the texture's palette, in order - slot `N` in the PNG is color `N` in the strip. Import the strip's colors as the texture PNG's colormap (any palette-aware editor can; GIMP imports a palette from an image, Aseprite loads one from a file). The grayscale art then snaps into full color for you to paint on. Two rules: stay in indexed-color mode, and keep the slots where they are. Paint with the existing entries rather than reordering or appending, since only the indices survive the round trip. (A CI texture saved un-paletted at its original size no longer matches its manifest format, and Retro skips it with a console note.)
- **Skipping palettes entirely: upscale it.** Resize the texture (2x is plenty) and paint in full RGBA. A resized texture takes the HD path below, where the image ships as true color and the palette machinery isn't consulted at all. Use it for any substantial redraw.

**Some model palettes can't be repainted.** A few models load 256 colors for a texture that lists 16, and only the model's own data holds the other 240, so those keep the model's palette. Repainting that `_TLUT` changes nothing in game, and nothing warns you. **For those, upscale instead.**

**Player colors override Banjo and Kazooie's palettes.** While any color is turned on in the Cosmetics Editor, those models draw with its palettes instead of your `_TLUT`s. Upscaled textures don't use a palette, so they keep your colors.

### HD textures

Any texture can be replaced at **higher than native resolution**. Redraw it at 2x, 4x or whatever your artwork needs, keeping the original aspect ratio; whole-number multiples keep the pixels lined up with how the game tiles the original. Retro notices the new size and ships the image in full color, and the game stretches it over the original texture's space. This works for every format.

Fonts and HUD sprites work too. The dialog and bold fonts are ordinary sprites, so an HD font sheet goes in the pack like any other texture. A language pack's own font is covered in the [language pack guide](LANGUAGE%20PACKS.md#5-fonts-and-glyphs); an HD sheet for it goes at `alt/assets/lang/<region>/...` and draws over the pack's SD one.

---

## 3. Pack it

Back in Retro: **Create OTR / O2R -> Replace Textures**, and this time answer **Yes**, selecting your `bk/` folder.

On that screen, before **Stage Textures**, turn on **"Prepend `alt/`"**. This prefixes every entry with `alt/`, and you want it for two reasons:

1. **It makes the pack toggleable.** With **Alternate Assets** on (the default), Lighthouse looks for `alt/` plus a texture's path first and falls back to the original. Players can switch the whole pack on and off from the mod menu without restarting.
2. **It fails safe.** If an entry is ever malformed, an `alt/` lookup falls back to the stock texture. A replacement at the *bare* path instead shadows the original outright - if it doesn't parse, that texture is gone from the game.

Then **Stage Textures**. Retro stages every changed texture; the bottom bar's **Finalize OTR / O2R** opens the review list, and **Generate OTR / O2R** writes the file. Save it as `<yourpack>.o2r`. Keep the `.o2r` extension - it's the only one the mod menu discovers. (**Compress Files**, beside the `alt/` toggle, is optional and off by default; enabling it just makes the file smaller.)

---

## 4. Install it

Drop the `.o2r` anywhere under Lighthouse's `mods/` folder (subfolders count too; Lighthouse creates the folder next to `bk.o2r` on first run). The pack appears in the in-game **Mods** menu, enabled by default.

When multiple mods replace the same texture, **the later archive wins**. That holds between entries at the same path: any `alt/` entry outranks a bare one, whatever the order. The menu lets you drag enabled mods into the order you want; a few placements are fixed around that:

| Loads | What |
|---|---|
| first | `bk.o2r` (base game) |
| then | enabled mods, in menu order |
| then | loose folders under `mods/` (unpacked archives, a development convenience) |
| last | `mods/~lang/` language packs (always active, never listed in the menu) |

While a **romhack** is active, plain mods are skipped - a pack that should apply under romhacks too belongs in `mods/~shared/`, which always loads. The **Alternate Assets** toggle governs every `alt/` entry from every mod at once; with it off, no `alt/` texture loads.

---

## 5. Advanced: additive art and externally-named packs

*(Everything in this section came with the BK64 additions, so it needs a current Retro build. Build from source if a release lacks them.)*

**Additive textures.** Lighthouse also looks under `alt/` for some art that no archive ships. Put a file at one of these paths and it gets used:

| Path shape | What it adds |
|---|---|
| `assets/sprite/<chunk>_<BLUE\|GREEN\|ORANGE\|PURPLE\|YELLOW>` | full-color art per Jinjo color, replacing the runtime tint of the shared gray head sprite |
| `assets/boldfont/<mask chunk>_<sphere hex id>` | a pre-textured bold-font glyph for one world sphere, replacing the runtime mask-plus-fill composite |
| `assets/<model\|level>/<asset>_tex_<i>_frame_<k>` | frame `k` of an animated texture; frame 0 is `_tex_<i>` itself |
| `assets/lang/<region>/<asset path>` | a variant used only while that language is active |

Retro knows these shapes. Give a file a matching name (or alias it onto the path, as below) and Retro will stage it even though the manifest has no entry for it, borrowing the size and format of the texture the path is based on. Additive art always ships as true-color raw, so draw it at a clean integer multiple of that texture's size.

Watch out with animated textures, because some are listed as a single strip holding every frame. Furnace Fun's bulb border, for example, is listed as 64x96 but animates three 64x32 frames. For textures like that, draw each frame at the top of a canvas the full listed size.

**Font glyph art.** Draw each glyph at a whole multiple of either its drawn tile or its full chunk, and Retro pads it onto the right canvas for you.

**`aliases.json`.** To repack a pack whose files aren't named by archive path (an emulator-era Rice-named pack, say), drop an `aliases.json` next to `manifest.json`, mapping each source image - path relative to the folder, extension included - onto its archive target; use a list when one image feeds several targets:

```json
{
  "Areas/Gobi's Valley/Banjo-Kazooie#00A0F146#0#3_all.png": "assets/level/ASSET_1476_GV_MEMORY_GAME_OPA_tex_4",
  "UI/Jinjos/Banjo-Kazooie#B36AB2E1#0#2_all.png": "assets/sprite/ASSET_7DF_UNNAMED_0_0_BLUE"
}
```

Copy the pack's files into the extraction folder, add the map, and the normal repack path handles the rest - aliased targets follow every rule above, additive shapes included. Retro reads only `.png`, `.jpg` and `.jpeg`, so convert a DDS pack first. When two images supply the same target, the first one GLideN64 would load is used: depth first, each folder in name order, skipping names that start with `.` or `~!~`.

---

## Gotchas

- **Never put a plain PNG at a game texture path.** Game textures are binary resources; Retro converts your PNGs into them at pack time. A raw `.png` file placed in an archive by hand doesn't parse - harmless under `alt/` (falls back), asset-destroying at the bare path.
