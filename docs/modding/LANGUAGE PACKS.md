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

This is the one edit the pack flow makes to the base game's config, and it also redirects the normal game build - so **revert it once your pack is built**.

Now run the export:

```
torch modding export <baserom.z64> -s <lighthouse> -d <workdir>
```

This decodes every asset into an editable form under `<workdir>/src/assets/assets/` and writes a `modding.yml` manifest listing them. The text you'll be translating lives in three folders, all under the base ROM's region (`us` for a US cartridge):

| Folder             | Contents                      | Count (US) |
|--------------------|-------------------------------|-----------:|
| `lang/us/dialog/`  | NPC and story dialog boxes    | 748 |
| `lang/us/quizq/`   | Grunty furnace quiz questions | 170 |
| `lang/us/gruntyq/` | Final-boss Grunty questions   |  30 |

You only edit the files you want to translate. Anything you leave alone stays in English. Next, edit the country code in **three places**:

1. the folder name - `lang/us/` -> `lang/es/` (`es` for this example);
2. the top-level key inside each yaml you keep - `lang/us/dialog/ASSET_…` -> `lang/es/dialog/ASSET_…`;
3. both sides of every `modding.yml` entry - `assets/lang/us/…: assets/lang/us/….yaml` -> `assets/lang/es/…`.

Then delete the asset yamls you won't be modifying and prune their `modding.yml` entries to match.

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

Translate the quoted text and leave the control code alone - that leading byte chooses the speaker's portrait and box behaviour (`0x80`+ are portrait codes; small values like `0x4` are flow/clear markers), and retyping it by hand will desync the box. If your translation needs more or fewer boxes than the original, add or remove `[code, "text"]` rows freely; the importer recounts them for you.

That `+` in `COMPA+ERO` isn't a typo: the quoted text is **not** plain Unicode, it's a sequence of font-sheet slots. This pack draws the `+` slot as `Ñ`, so it renders as *COMPAÑERO*. The next section explains this properly.

### Quiz and Grunty questions

These have the same idea, split into the question `Text` and the three answer `Options`:

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

Translate the question lines and the answer text. On each option, **keep the two bytes before the string** (`0xfd, 0x6c`) as they bind the answer to its slot; the importer rejects an option row without them. Grunty questions (`gruntyq/`) use the same shape. (Here `$` and `%` are font slots this pack draws as `¿` and `Á` once its font is in place; see below.)

One yaml hygiene rule: always put double quotes around your translations - an unquoted `,`, `:` or `#` breaks the yaml when the pack is imported.

### How the text is encoded

The quoted strings are sequences of **font-sheet slots**, not UTF-8. Every byte is an index into the game's font, so what you type is a *slot number*; it just happens to look like ASCII because the dialog font draws `A–Z`, `0–9`, and common punctuation in their ASCII positions. Type a literal `ñ` (UTF-8) into the yaml and you'll get garbage; you have to type the slot the font draws as `Ñ`.

The dialog/quiz/grunty font (`0x6EB`) is a flat, contiguous sheet beginning at byte `0x21`. Byte `0x21` is the first glyph, `0x22` the second, and so on. The stock font holds 62 glyphs, so the reachable range is `0x21`–`0x5E`:

```
! " # $ % & ' ( ) * + , - . /   0-9   : ; < = > ? @   A-Z   [ \ ] ^
```

The stock font holds only the ASCII set - `A–Z`, `0–9`, and punctuation. **None of Spanish's accents are in it**: `¡ ¿ Á É Í Ó Ú Ñ` all live in byte positions the stock font draws as ASCII symbols (`#`, `%`, `&`, …). So each accent has to be **added** to the font, and you type it as whatever slot you put it in. Our Spanish pack repaints eight symbol slots - `#` `$` `%` `&` `(` `)` `*` `+` → `¡` `¿` `Á` `É` `Í` `Ó` `Ú` `Ñ` (full table in [#5](#5-fonts-and-glyphs)). That's why `COMPA+ERO` renders *COMPAÑERO* (`+` = Ñ) and `$EST%S` renders *¿ESTÁS*. A `\xNN` escape works for any byte inside a **single-quoted** string (`'\xFD'` style - the importer decodes them, and it's the form the exporter itself emits for text carrying inline control codes). Use it for slots past `0x7E`, an extended sheet's high glyphs included; a literal backslash in such a string is `\\`. Don't put high escapes in *double*-quoted strings - there the yaml parser processes them itself, which is fine up to `\x7F` but re-encodes `\x80`–`\xFF` as two UTF-8 bytes, desyncing the text.

The same goes for lowercase `a–z` and any non-Latin script - no stock glyph, so you add one. There are two ways to add glyphs, both covered in [#5 Fonts and glyphs](#5-fonts-and-glyphs):

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

Torch turns this into the `langinfo` manifest that Lighthouse reads to build its language menu. Because it lives in your pack's source, the language list never touches the base game's asset yamls. The fields:

- **`name`** - what shows in the options menu. This label is drawn by the PC menu (not the in-game font), so the native spelling with real accents is fine here.
- **`index`** - the language's slot; `0` for a single-language pack.
- **`script`** - which of the game's two text paths to use. `0` (Latin) draws from the normal dialog/bold font sheets (`0x6EB`/`0x6EC`), which is what every Western release uses, so almost every pack - Spanish included - wants `0`. `1` (Japanese) switches to the path the original JP cartridge used: a separate dialog font (`0x6EA`) plus pre-rendered world-name banners, because there are far too many kanji to slot-map onto a Latin sheet. Only use `1` if your pack reuses that JP font, and if you do you must include `0x6EA`.
- **`region`** (optional) - the folder your assets live under inside the finished `.o2r` (`lang/<region>/`). Defaults to the cartridge region; we set `es` so a Spanish pack doesn't collide with others.
- **`strings`** (optional) - translations for hardcoded UI text the asset pipeline can't reach; see the next section.

With no `langinfo.yml` at all, the build names the language after the cartridge region (US → `English (US)`). That fallback is exactly how the retail `bkpal` / `bkjp` packs are produced.

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

…which produces `PARTIDA 1: TIEMPO 0:12:34,` / `5 PIEZAS, 100 NOTAS`.

One limit comes with this approach: **word order is fixed by the engine.** The number always precedes its noun and the prefix always precedes the game number, so a language that needs the count *after* the noun can't reorder it here. The numbers and the clock are rendered by the engine and aren't translatable - only the words around them are. And the rebuild only happens for a pack that actually supplies these `strings:`; a base US/PAL game keeps its own built-in line.

### Pack mode

The `dialog_pack: true` flag was already set back in [step 1](#1-export-the-text-to-editable-yaml) - the export needs it too. At import time it's what trims the build down to just the translated assets, prefixes them under `lang/<region>/`, and builds the `langinfo` manifest.

---

## 4. Build the pack

```
torch modding import o2r <baserom.z64> -s <lighthouse> -d <workdir>
```

The importer walks the asset list: your edited yamls are re-encoded from `<workdir>`, everything else parses straight from the ROM, and `dialog_pack` strips the result down to the language assets plus `langinfo`. The finished pack lands in **`<workdir>/mods/~lang/`** automatically, named from your `output.binary` - `bkes.o2r` in our case. Copy that into Lighthouse's `mods/~lang/` folder and it shows up in the options menu.

---

## 5. Fonts and glyphs

If your translation only uses characters the stock font already draws - uppercase `A–Z`, digits, and punctuation - you're finished at step 4. The moment you need anything else (any accent, lowercase, or a non-Latin script), you ship a **replacement font sheet**. BK keeps its fonts as CI8 sprites and draws text with two of them:

| Asset   | Sprite                               | Used for |
|---------|--------------------------------------|----------|
| `0x6EB` | `SPRITE_DIALOG_FONT_ALPHAMASK`       | dialog / quiz / grunty text |
| `0x6EC` | `SPRITE_BOLD_FONT_LETTERS_ALPHAMASK` | world-name titles, bold headers |

`dialog_pack` carries both font masks automatically, so you just edit the exported font asset through the normal modding flow and redraw the slots you're repurposing. Cover `0x6EB` and your dialog comes across; cover `0x6EC` as well and the bold headers do too.

The stock font has none of Spanish's accents, so our pack repaints eight unused symbol slots in `0x6EB`, redrawing each as the glyph we want:

| Glyph   | ¡   | ¿   | Á   | É   | Í   | Ó   | Ú   | Ñ   |
|---------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Type as | `#` | `$` | `%` | `&` | `(` | `)` | `*` | `+` |
| Slot    |`0x23`|`0x24`|`0x25`|`0x26`|`0x28`|`0x29`|`0x2A`|`0x2B`|

Each repainted glyph is just a base letter with an accent added - `Á` is the `A` glyph with an acute stroke, `Ñ` is `N` with a tilde, `¿` is the `?` glyph flipped, and so on. That's why the strings above read `$EST%S SEGURO?` (`¿ESTÁS SEGURO?`) - each accented letter is the symbol slot whose glyph we redrew. A pack for a whole new alphabet would instead *extend* the sheet (add glyphs past `0x5E`), as described in [#2's encoding notes](#how-the-text-is-encoded); a Russian pack, for instance, draws the full Cyrillic set into new slots rather than repainting a handful.

The swap is live: glyphs apply both when the pack is chosen at boot and when you switch to it from the options menu, because the runtime re-decodes the font slots on every language change (and picks up an extended sheet's larger glyph count at the same time). Switching back to a built-in language restores the originals.

Everything you replace here is **SD**. High-resolution versions are a separate, optional layer - see [#7](#7-hd-fonts--textures-optional).

---

## 6. Region-only content (additive assets)

A little text exists in the PAL/JP versions but **not** in US v1.0, so the US export has no file for it. The clearest case is the **Motzand parade credit** (canonical dialog `0x11CA`): PAL and JP move Motzand into the Furnace-Fun parade, and the dialog shown in his slot is the game's *localization-team credit* — the heading "WORD SWOPPING" followed by the translators' names. US v1.0 never had that slot, so it never populates that dialog: its parade-credit dialogs run contiguously from `0x11AF` to `0x11C9`, and `0x11CA` is simply the first empty id past the end. That empty slot is what your pack fills. (PAL/JP keep the same credit at their own ids — PAL `0xBF9`, for instance — and the language system re-points `0x11CA` to it on those bases.)

You can still ship that content by declaring it as an **additive** asset - a slot the base ROM leaves empty that your pack fills:

1. **Create the yaml** under the matching folder, named `ASSET_<hexid>_<label>.yaml`, with the normal structure. This is the natural place to credit your translation team:

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

2. **Register it in `modding.yml`** (the export won't have, since the slot was empty). Add one line under `assets:`, with the full `assets/…` path as both key and value:

   ```yaml
     assets/lang/es/dialog/ASSET_11CA_MOTZAND: assets/lang/es/dialog/ASSET_11CA_MOTZAND.yaml
   ```

On import, `dialog_pack` scans your `modding.yml` for any `lang/<region>/{dialog,quizq,gruntyq,sprite}/ASSET_<id>` the base table didn't provide and folds it into the pack. Use the **canonical (v1.0) id** for the slot (`0x11CA` for the Motzand credit) no matter which region your base ROM is.

This matters for the parade because Lighthouse keys the alternate (PAL/JP-style) parade on the *presence* of `0x11CA` in the active pack, not on the base version. A player on a plain US v1.0 `bk.o2r` who loads a pack that supplies `0x11CA` gets the Motzand parade order **and** the translated credit, while the rest of the game keeps using US assets. Cover `0x11CA` and the parade switches; omit it and the US parade is left untouched.

### World-name banners

The `sprite` folder covers the pause-menu **world-name banners** - the pre-rendered titles the JP release draws on its totals pages instead of bold-font text. There are twelve, one per menu page, and a pack ships them as additive sprites at `0x1600 + page`:

| Id | Page | Id | Page |
|---|---|---|---|
| `0x1600` | Totals overview | `0x1606` | Bubblegloop Swamp |
| `0x1601` | Spiral Mountain | `0x1607` | Freezeezy Peak |
| `0x1602` | Gruntilda's Lair | `0x1608` | Gobi's Valley |
| `0x1603` | Mumbo's Mountain | `0x1609` | Mad Monster Mansion |
| `0x1604` | Treasure Trove Cove | `0x160A` | Rusty Bucket Bay |
| `0x1605` | Clanker's Cavern | `0x160B` | Click Clock Wood |

Each page is checked on its own: ship any subset, and a page without a banner keeps drawing its name as bold-font text. A `script: 0` pack can banner just the handful of names its repainted font can't spell and leave the rest to text. Declare each one like any other additive asset (the yaml under `sprite/`, plus its `modding.yml` line).

The sprite itself is a single-frame **RGBA32 intensity mask** - the same format as the JP originals. Draw the title as white-on-transparent artwork, not pre-coloured text: at draw time the runtime refills the mask live with the world's swirling bold-font sphere texture (the highlighted current-world page gets its own sphere; other pages the default fill). The banner renders centered on the page header at its native size and is only scaled down if it's wider than ~92% of the screen, so any sensible resolution works.

(A `script: 1` pack that reuses the JP cart's own banners keeps their native ids - `0xE2C`–`0xE37` re-point to the `0x1600` slots automatically. `0xE38` is not a banner; it's the default fill texture the masks are filled with.)

---

## 7. HD fonts & textures (optional)

Everything above produces an **SD** pack. Players running an HD texture pack need high-resolution versions of whatever you replaced, and those are made with a different tool: [Retro](https://github.com/HarbourMasters/retro), HarbourMasters' OTR/O2R generation tool.

The two tiers live at different paths inside the o2r, scoped by language:

| Tier | Path in the o2r              | Built with        |
|------|------------------------------|-------------------|
| SD   | `assets/lang/<region>/…`     | Torch (steps 1–5) |
| HD   | `alt/assets/lang/<region>/…` | Retro             |

They compose automatically. When HD/alt assets are enabled, the resource loader tries `alt/` + the requested path first and falls back to the plain path. Because the language system has already re-pointed your asset to `assets/lang/<region>/…`, the loader transparently looks for `alt/assets/lang/<region>/…`, using the HD version when present and the SD one otherwise.

So **always ship SD**: it's the SD re-point that creates the `assets/lang/<region>/` path the HD layer keys off. A pack can be SD-only and work everywhere; HD is purely additive, for players who run an HD texture pack on top.
