# Making a Language Pack

A language pack is a small companion archive that adds a new in-game language alongside the retail `bkpal.o2r` / `bkjp.o2r`. Players drop it into their `mods/~lang/` folder and pick the new language from the options menu.

This guide walks through building one pack end to end: a **Spanish** translation, from the first export to a finished `bkes.o2r`.

---

## 1. Export the text to editable YAML

Pack mode has to be on **before** you export. In the repo's `config.yml`, find your base ROM's entry (keyed by ROM hash) and add the `dialog_pack` flag to its `config:` block, pointing `output.binary` at your pack's filename:

```yaml
config:
  dialog_pack: true   # add
  output:
    binary: bkes.o2r  # was bk.o2r; the pack's filename
```

This is the one edit the pack flow makes to the base game's config, and it also redirects the normal game build - so **revert it once your pack is built**. To leave the repo alone instead, copy `config.yml` and `assets/yaml/` into a folder of your own, make the edit there, and pass that folder as `-s`.

Now run the export:

```
torch modding export <baserom.z64> -s <lighthouse> -d <workdir>
```

This decodes every asset into an editable form under `<workdir>/src/assets/assets/` and writes a `modding.yml` manifest listing them. The text you'll be translating lives in three folders, all under the base ROM's region (`us` for a US cartridge):

| Folder             | Contents                      | Count (US) |
|--------------------|-------------------------------|-----------:|
| `lang/us/dialog/`  | NPC and story dialog boxes    | 748 |
| `lang/us/quizq/`   | Furnace Fun quiz questions    | 170 |
| `lang/us/gruntyq/` | Final-boss Grunty questions   |  30 |

Pack mode also exports `lang/us/model/` (the level-entry signs, jigsaw pictures, THE END sign and the title-screen overlays, for translating signage) and `lang/us/sprite/` (the two fonts, see [#5](#5-fonts-and-glyphs)).

You only edit the files you want to translate. Anything you leave alone stays in English. Next, edit the country code in **three places**:

1. the folder name - `lang/us/` -> `lang/es/` (`es` for this example);
2. the top-level key inside each YAML you keep - `lang/us/dialog/ASSET_...` -> `lang/es/dialog/ASSET_...`;
3. both sides of every `modding.yml` entry - `assets/lang/us/<asset>: assets/lang/us/<asset>.yaml` -> `assets/lang/es/<asset>: assets/lang/es/<asset>.yaml`.

Then delete the asset YAML files you won't be modifying and prune their `modding.yml` entries to match.

---

## 2. Edit the text

### Dialog

Each dialog file is a list of **boxes** split between a `Bottom` and `Top` screen region. A box is a `[control_code, "text"]` pair:

```yaml
lang/es/dialog/ASSET_A21_BLUBBER_MEET:
  Bottom:
    - [0x9f, "HOLA, COMPA+ERO. SOY EL TESORO DE BLUBBER."]   # HOLA, COMPAÑERO. (+ = Ñ)
    - [0x4,  ""]
  Top:
    - [0x4,  ""]
```

Translate the quoted text and leave the control code alone - that leading byte chooses the speaker's portrait and box behavior (`0x80`+ are portrait codes; small values like `0x4` are flow/clear markers), and retyping it by hand will desync the box. The [dialog guide](CUSTOM%20DIALOG.md) lists every code. If your translation needs more or fewer boxes than the original, add or remove `[code, "text"]` rows freely; the importer recounts them for you.

A PAL export also puts `French` and `German` maps in every text YAML. Delete both from the files you translate, or keep both: a file with only one is left out with an error.

That `+` in `COMPA+ERO` isn't a typo: the quoted text is **not** plain Unicode, it's a sequence of font-sheet slots. This pack draws the `+` slot as `Ñ`, so it renders as *COMPAÑERO*. The next section explains this properly.

### Quiz and Grunty questions

These work the same way, split into the question's `Text` and its three answer `Options`:

```yaml
lang/es/quizq/ASSET_1213_FF_QUIZ_QUESTION:
  Text:
    - [0x80, "YA SABES LO QUE HAY QUE HACER:"]
    - [0x80, "$CU%NTAS NOTAS PARA LA PRIMERA PUERTA?"]   # ¿CUÁNTAS NOTAS…?
  Options:
    - [0x81, 0xfd, 0x6c, "50"]
    - [0x82, 0xfd, 0x6c, "100"]
    - [0x83, 0xfd, 0x6c, "75"]
```

Translate the question lines and the answer text. On each option, **keep the two bytes before the string** (`0xfd, 0x6c`). PAL's quiz options have no such bytes and export as `[0x81, "50"]`. Any other row makes the importer log an error and leave the question out, but the import still finishes, so check the log. Grunty questions (`gruntyq/`) use the same shape. (Here `$` and `%` are font slots this pack draws as `¿` and `Á` once its font is in place; see below.)

One YAML rule: always put double quotes around your translations - an unquoted `,`, `:` or `#` breaks the YAML when the pack is imported. The exception is a string with `\xNN` escapes, which goes in single quotes (see below).

### How the text is encoded

The quoted strings are sequences of **font-sheet slots**, not UTF-8. Every byte is an index into the game's font, so what you type is a *slot number*; it just happens to look like ASCII because the dialog font draws `A-Z`, `0-9`, and common punctuation in their ASCII positions. Type a literal `ñ` (UTF-8) into the YAML and you'll get garbage; you have to type the slot the font draws as `Ñ`.

The dialog/quiz/grunty font (`0x6EB`) is a flat, contiguous sheet beginning at byte `0x21`. Byte `0x21` is the first glyph, `0x22` the second, and so on. The stock font holds 62 glyphs, so the reachable range is `0x21`-`0x5E`:

```
! " # $ % & ' ( ) * + , - . /   0-9   : ; < = > ? @   A-Z   [ \ ] ^
```

**The stock font has none of Spanish's accents.** There's no slot for `¡ ¿ Á É Í Ó Ú Ñ`, so you have to **add** each one to the font yourself and then type whichever slot you put it in. Our Spanish pack repaints eight symbol slots, turning `#` `$` `%` `&` `(` `)` `*` `+` into `¡` `¿` `Á` `É` `Í` `Ó` `Ú` `Ñ` (full table in [#5](#5-fonts-and-glyphs)), which is why `COMPA+ERO` comes out as *COMPAÑERO* and `$EST%S` as *¿ESTÁS*.

To type a slot past `0x7E`, such as an extended sheet's high glyphs, use a `\xNN` escape inside a **single-quoted** string (`'\xFD'`). The importer decodes these, and it's the same form the exporter writes for text with inline control codes. A literal backslash in a string like that is written `\\`. Keep high escapes out of *double*-quoted strings, though: there the YAML parser decodes them itself, which works up to `\x7F` but turns `\x80`-`\xFF` into two UTF-8 bytes and throws the text out of step.

The same goes for lowercase `a-z` and any non-Latin script - no stock glyph, so you add one. There are two ways to add glyphs, both covered in [#5 Fonts and glyphs](#5-fonts-and-glyphs):

- **Repaint a spare slot.** Pick a slot you don't otherwise use (a punctuation symbol like `%` or `&`) and redraw it as the letter you need. The reachable range stays the same; you're just changing what a slot *looks* like. This is how our Spanish pack gets its `¡ ¿` and acute vowels - a handful of repainted symbol slots.
- **Extend the sheet.** Add more glyphs to the font sprite. The reachable range grows with it: glyph #63 becomes byte `0x5F`, #64 `0x60`, and so on. Lighthouse reads the glyph count from *your* font and re-reads it on a live language switch, so the new bytes light up automatically. This is the route for a whole new alphabet - Cyrillic for a Russian pack, say, or Greek - where repainting a few symbols won't cut it.

Either way, you type the **slot**, and the replacement font makes it render. Pick one slot per character and stay consistent across every file. Our Spanish pack uses the repaint route; its full slot plan is in [#5](#5-fonts-and-glyphs).

---

## 3. Declare the language

Two small things turn an ordinary import into a language pack: the `dialog_pack` config flag you already set in [step 1](#1-export-the-text-to-editable-yaml), and a header that names the language.

### Name the language

Drop a `langinfo.yml` next to `modding.yml` in your pack's source dir (`<workdir>/src/assets/`):

```yaml
# langinfo.yml
region: es
langinfo:
  - { name: Español, index: 0, script: 0 }
strings: # optional - see "Hardcoded UI strings" below
  "<English UI string>": "<translation>"
```

Torch turns this into the `langinfo` manifest that Lighthouse reads to build its language menu. Because it lives in your pack's source, the language list never touches the base game's asset YAML files. The fields:

- **`name`** - what shows in the options menu. This label is drawn by the PC menu (not the in-game font), so the native spelling with real accents is fine here.
- **`index`** - the language's slot; `0` for a single-language pack.
- **`script`** - which of the game's two text paths to use. `0` (Latin) draws from the normal dialog/bold font sheets (`0x6EB`/`0x6EC`), which is what every Western release uses, so almost every pack - Spanish included - wants `0`. `1` (Japanese) switches to the JP cartridge's text path: its dialog font (`0x6EA`), the kana file-select prompts, save-info line and parade. Use it for a pack built from a Japanese ROM; a `script: 1` pack missing `0x6EA` is rejected at load. [World-name banners](#world-name-banners) work under `script: 0`.
- **`region`** (optional) - the folder your assets live under inside the finished `.o2r` (`lang/<region>/`). Defaults to the cartridge region; we set `es` so a Spanish pack doesn't collide with others.
- **`strings`** (optional) - translations for hardcoded UI text the asset pipeline can't reach; see the next section.

With no `langinfo.yml` at all, the build names the language after the cartridge region (US -> `English (US)`). That fallback is how the retail `bkpal` / `bkjp` packs are produced.

### Hardcoded UI strings

Some on-screen text isn't a ROM asset - it's a string baked into the engine: the pause-menu options, the file-select prompts, the file-info box. The asset pipeline can't see these, so you translate them through the `strings:` map instead. The key is the **exact English string** the engine uses; the value is your translation, in the same slot encoding as dialog text. Anything you don't list keeps its English text. These are the keys available:

| Where | English keys |
|-------|--------------|
| Pause menu | `"RETURN TO GAME"`, `"EXIT TO WITCH'S LAIR"`, `"VIEW TOTALS"`, `"SAVE AND QUIT"` |
| Save-and-quit confirm | `"ARE YOU SURE?"`, `"A - YES, B - NO"` |
| File-select prompts | `"USE THE CONTROL STICK TO SELECT A GAME."`, `"PRESS A TO PLAY THE GAME OR Z TO ERASE IT!"`, `"ARE YOU SURE? PRESS A TO CONFIRM, OR B TO CANCEL."` |
| File-select info box | the runtime-assembled line - see [Lines that mix in live values](#lines-that-mix-in-live-values) |

For our Spanish pack:

```yaml
strings:
  # Pause menu
  "RETURN TO GAME": "VOLVER AL JUEGO"
  "EXIT TO WITCH'S LAIR": "SALIR A LA GUARIDA"
  "VIEW TOTALS": "VER TOTALES"
  "SAVE AND QUIT": "GUARDAR Y SALIR"
  "ARE YOU SURE?": "$EST%S SEGURO?"          # ¿ESTÁS SEGURO?
  "A - YES, B - NO": "A - S(, B - NO"         # A - SÍ, B - NO
  # File-select prompts
  "USE THE CONTROL STICK TO SELECT A GAME.": "USA EL STICK PARA ELEGIR UNA PARTIDA."
  "PRESS A TO PLAY THE GAME OR Z TO ERASE IT!": "PULSA A PARA JUGAR O Z PARA BORRAR."
  "ARE YOU SURE? PRESS A TO CONFIRM, OR B TO CANCEL.": "$SEGURO? PULSA A PARA CONFIRMAR O B PARA CANCELAR."  # ¿SEGURO?…
```

(`$` `%` `(` are the spare slots this pack repaints as `¿` `Á` `Í`; see [#5](#5-fonts-and-glyphs).)

The **world-name titles** on the totals pages aren't in this map. They're drawn as graphics whose layout is tuned per name, so you translate them by repainting the bold font (`0x6EC`) or shipping banner sprites - see [#5](#5-fonts-and-glyphs) and [#6](#6-region-only-content-additive-assets) - not through `strings:`.

#### Lines that mix in live values

The file-select info box reads, in English, something like:

```
GAME 1: TIME 0:12:34,
5 JIGSAWS, 100 NOTES.
```

That line is assembled at runtime from the save file - game number, play time, jiggy and note totals - so there's no single fixed string to translate. Instead you translate it **one static fragment at a time** and let the engine splice the live numbers back in. The two nouns come in separate singular and plural forms, chosen by the count, so each language gets the plural it actually needs:

| English key  | Where it appears |
|--------------|------------------|
| `"GAME "`    | prefix before the game number |
| `": TIME "`  | label before the clock |
| `": EMPTY"`  | shown instead, for an empty file |
| `" JIGSAW"`  | jiggy noun, count of 1 |
| `" JIGSAWS"` | jiggy noun, count ≠ 1 |
| `" NOTE"`    | note noun, count of 1 |
| `" NOTES"`   | note noun, count ≠ 1 |

For Spanish:

```yaml
strings:
  "GAME ": "PARTIDA "
  ": TIME ": ": TIEMPO "
  ": EMPTY": ": LIBRE"
  " JIGSAW": " PIEZA"
  " JIGSAWS": " PIEZAS"
  " NOTE": " NOTA"
  " NOTES": " NOTAS"
```

With those in place, the line comes out as `PARTIDA 1: TIEMPO 0:12:34,` / `5 PIEZAS, 100 NOTAS`.

One limit comes with this approach: **word order is fixed by the engine.** The number always precedes its noun and the prefix always precedes the game number, so a language that needs the count *after* the noun can't reorder it here. The numbers and the clock are rendered by the engine and aren't translatable - only the words around them are. And the rebuild only happens for a pack that actually supplies these `strings:`; a base US/PAL game keeps its own built-in line.

### Pack mode

The `dialog_pack: true` flag was already set back in [step 1](#1-export-the-text-to-editable-yaml) - the export needs it too. At import time it's what trims the build down to just the translated assets, prefixes them under `lang/<region>/`, and builds the `langinfo` manifest.

---

## 4. Build the pack

```
torch modding import o2r <baserom.z64> -s <lighthouse> -d <workdir>
```

The importer walks the asset list: your edited YAML files are re-encoded from `<workdir>`, everything else parses straight from the ROM, and `dialog_pack` strips the result down to the language assets plus `langinfo`. The finished pack lands in **`<workdir>/mods/~lang/`** automatically, named from your `output.binary` - `bkes.o2r` in our case. Copy that into Lighthouse's `mods/~lang/` folder and it shows up in the options menu.

---

## 5. Fonts and glyphs

If your translation only uses characters the stock font already draws - uppercase `A-Z`, digits, and punctuation - you're finished at step 4. The moment you need anything else (any accent, lowercase, or a non-Latin script), you ship a **replacement font sheet**. BK draws text with two font sprites:

| Asset                                   | Format | Used for |
|-----------------------------------------|--------|----------|
| `ASSET_6EB_DIALOG_FONT_ALPHAMASK`       | IA8    | dialog / quiz / grunty text |
| `ASSET_6EC_BOLD_FONT_LETTERS_ALPHAMASK` | RGBA32 | world-name titles, bold headers |

`dialog_pack` carries both font masks automatically, so you just edit the exported font asset through the normal modding flow and redraw the slots you're repurposing. Cover `0x6EB` and your dialog comes across; cover `0x6EC` as well and the bold headers do too.

Pack mode writes each font's YAML but not its glyph PNGs. Run a normal export (without `dialog_pack`) once and copy the font's PNGs, such as `ASSET_6EB_DIALOG_FONT_ALPHAMASK_0_0.png`, from its `assets/sprite/` into your pack's `sprite/` folder beside the YAML.

The stock font has none of Spanish's accents, so our pack repaints eight unused symbol slots in `0x6EB`, redrawing each as the glyph we want:

| Glyph   | ¡   | ¿   | Á   | É   | Í   | Ó   | Ú   | Ñ   |
|---------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Type as | `#` | `$` | `%` | `&` | `(` | `)` | `*` | `+` |
| Slot    |`0x23`|`0x24`|`0x25`|`0x26`|`0x28`|`0x29`|`0x2A`|`0x2B`|

Each repainted glyph is just a base letter with an accent added - `Á` is the `A` glyph with an acute stroke, `Ñ` is `N` with a tilde, `¿` is the `?` glyph flipped, and so on. A pack for a whole new alphabet would instead *extend* the sheet (add glyphs past `0x5E`), as described in [#2's encoding notes](#how-the-text-is-encoded); a Russian pack, for instance, draws the full Cyrillic set into new slots rather than repainting a handful.

The swap is live: your glyphs show up whether the pack is picked at boot or switched to from the options menu, and switching back to a built-in language restores the originals.

Everything you replace here is **SD**. High-resolution versions are a separate, optional layer built with Retro - see the [texture pack guide](TEXTURE%20PACKS.md); an HD sheet ships at `alt/assets/lang/<region>/...` and composes on top of your SD pack automatically.

---

## 6. Region-only content (additive assets)

A little text exists in the PAL/JP versions but **not** in US v1.0, so the US export has no file for it. The clearest case is the **Motzand parade credit** (dialog `0x11CA`): PAL and JP move Motzand into the Furnace-Fun parade, and the dialog shown in his slot is the game's *localization-team credit* - the heading "WORD SWOPPING" followed by the translators' names. US never had that slot: its parade-credit dialogs run contiguously from `0x11AF` to `0x11C9`, and `0x11CA` is the first empty id past the end. That empty slot is what your pack fills.

You can still ship that content by declaring it as an **additive** asset - a slot the base ROM leaves empty that your pack fills:

1. **Create the YAML** under the matching folder, named `ASSET_<hexid>_<label>.yaml`, with the normal structure. This is the natural place to credit your translation team:

   ```yaml
   # assets/lang/es/dialog/ASSET_11CA_MOTZAND.yaml
   lang/es/dialog/ASSET_11CA_MOTZAND:
     Bottom:
       - [0xb9, "WORD SWOPPING"]      # the credit heading, as in PAL/JP
       - [0xb9, "TU NOMBRE AQUI"]     # your translators, one per line
       - [0x4,  ""]
     Top:
       - [0x4,  ""]
   ```

2. **Register it in `modding.yml`** (the export won't have, since the slot was empty). Add one line under `assets:`, with the full `assets/...` path as both key and value:

   ```yaml
     assets/lang/es/dialog/ASSET_11CA_MOTZAND: assets/lang/es/dialog/ASSET_11CA_MOTZAND.yaml
   ```

On import, `dialog_pack` scans your `modding.yml` for any `lang/<region>/{dialog,quizq,gruntyq,sprite}/ASSET_<id>` the base table didn't provide and folds it into the pack. The ids are the same whichever region your base ROM is.

On a US base, the parade switches to the PAL/JP order when the active pack supplies `0x11CA`, with your credit in Motzand's slot. The rest of the game keeps using US assets.

### World-name banners

The `sprite` folder covers the pause-menu **world-name banners** - the pre-rendered titles the JP release draws on its totals pages instead of bold-font text. There are twelve, one per menu page, plus the fill they're colored from. Ship any subset you like; a page without a banner just keeps drawing its name as bold-font text.

| Asset | Page |
|---|---|
| `ASSET_1600_WORLD_NAME_TOTAL` | Totals overview |
| `ASSET_1601_WORLD_NAME_SPIRAL_MOUNTAIN` | Spiral Mountain |
| `ASSET_1602_WORLD_NAME_GRUNTILDAS_LAIR` | Gruntilda's Lair |
| `ASSET_1603_WORLD_NAME_MUMBOS_MOUNTAIN` | Mumbo's Mountain |
| `ASSET_1604_WORLD_NAME_TREASURE_TROVE_COVE` | Treasure Trove Cove |
| `ASSET_1605_WORLD_NAME_CLANKERS_CAVERN` | Clanker's Cavern |
| `ASSET_1606_WORLD_NAME_BUBBLEGLOOP_SWAMP` | Bubblegloop Swamp |
| `ASSET_1607_WORLD_NAME_FREEZEEZY_PEAK` | Freezeezy Peak |
| `ASSET_1608_WORLD_NAME_GOBIS_VALLEY` | Gobi's Valley |
| `ASSET_1609_WORLD_NAME_MAD_MONSTER_MANSION` | Mad Monster Mansion |
| `ASSET_160A_WORLD_NAME_RUSTY_BUCKET_BAY` | Rusty Bucket Bay |
| `ASSET_160B_WORLD_NAME_CLICK_CLOCK_WOOD` | Click Clock Wood |
| `ASSET_160C_BOLD_FONT_FILL_TEXTURE` | the fill, shared by all twelve |

#### 1. Draw the title

Draw it as an RGBA32 PNG, with the letter shapes in the **alpha** channel and the shading in the **blue** channel. Two rules:

- **Blue is the brightness.** The game recolors the title as it draws, and it works out how light each pixel should be from the blue channel alone. Bright blue is a lit pixel, dark blue is a shadowed one.
- **Shade it like the bold font.** Flat, fully-bright letters give you a flat title with no outline. The outline comes from darkening the outer pixel or two of every letter; the modeling comes from shading the rest.

Size: **32 pixels tall**, up to about 320 wide. Anything wider is scaled down to fit the screen.

#### 2. Draw the fill

One 32x32 PNG, **fully opaque** - no transparency anywhere, right out to the edges. This is the color your titles get painted with.

It is stretched across the whole width of a title, so the left and right edges are the parts a wide title shows most. A soft swirl or gradient that looks good edge to edge works well; the JP one is a cyan swirl.

#### 3. Add them to the pack

Two files per banner: the YAML under `sprite/`, and the PNG named `<asset>_0_0.rgba32.png` (frame 0, chunk 0):

```yaml
# assets/lang/es/sprite/ASSET_1600_WORLD_NAME_TOTAL.yaml
lang/es/sprite/ASSET_1600_WORLD_NAME_TOTAL:
  FrameCount: 1
  FormatCode: 2048   # 0x800 = RGBA32
  Frames:
    - ChunkCount: 1
      Chunks:
        - {X: 0, Y: 0}
```

Copy that verbatim for every banner; only the name changes. `{X: 0, Y: 0}` is where the image sits, not how big it is; the size is read from your PNG. This is the older sprite schema, which Torch still reads; the exporter writes the newer one (`Format`, `DisplaySize`, and `Frames` whose `Chunks` are `{Position, Image}`). Both `modding.yml` lines:

```yaml
  assets/lang/es/sprite/ASSET_1600_WORLD_NAME_TOTAL: assets/lang/es/sprite/ASSET_1600_WORLD_NAME_TOTAL.yaml
  assets/lang/es/sprite/ASSET_1600_WORLD_NAME_TOTAL_0_0: assets/lang/es/sprite/ASSET_1600_WORLD_NAME_TOTAL_0_0.rgba32.png
```

The YAML's top-level key, both `modding.yml` keys and the PNG's `_0_0` stem all have to spell the name the same way. And `lang/<region>/` has to match the `region` from your `langinfo.yml`.

The fill is declared the same way, but as RGBA16, so `FormatCode: 1024`:

```yaml
# assets/lang/es/sprite/ASSET_160C_BOLD_FONT_FILL_TEXTURE.yaml
lang/es/sprite/ASSET_160C_BOLD_FONT_FILL_TEXTURE:
  FrameCount: 1
  FormatCode: 1024   # 0x400 = RGBA16
  Frames:
    - ChunkCount: 1
      Chunks:
        - {X: 0, Y: 0}
```

It needs the same two `modding.yml` lines, with the PNG named `ASSET_160C_BOLD_FONT_FILL_TEXTURE_0_0.rgba16.png`.

If a title needs nudging off-center, add a `FrameHeader` to the frame - `x`/`y` shift it:

```yaml
    - ChunkCount: 1
      FrameHeader: {x: 0, y: -4}
      Chunks:
        - {X: 0, Y: 0}
```
