# Banjo-Kazooie Version Differences

A detailed comparison of what Rare changed between the US v1.0, US v1.1, and PAL releases of Banjo-Kazooie, based on binary analysis of all three ROMs.

## At a Glance

v1.1 is a bug-fix revision. Rare cleaned up textures, fixed level geometry, rewrote every Furnace Fun question, stripped leftover debug assets, and reorganized the asset table into a tighter layout. PAL and JP are both based on v1.1 with all the same fixes — PAL adds multi-language dialog support, JP adds Japanese text and kanji font sheets.

| | US v1.0 | US v1.1 | PAL | JP |
|---|---|---|---|---|
| Game code | NBKE | NBKE | NBKP | NBKJ |
| Assets | 3314 | 3044 | 3059 | 3065 |
| Max asset ID | 5570 | 3615 | 3616 | 3640 |
| Dialog format | Single-language | Single-language | 3 languages (EN/DE/FR) | Single-language (Japanese text) |
| Quiz questions | Original set | All rewritten | All rewritten (multi-lang) | All rewritten (Japanese) |
| Debug models | Present | Removed | Present but modified | Removed |
| Font system | FontSprite + stubs | Individual glyphs | Glyphs + accented chars | Glyphs + 13 kanji sheets |

---

## What Changed in v1.1

### Kazooie's Feather Texture (4 models)

Four models that feature Kazooie — both Banjo-Kazooie body models (low-poly and high-poly), Turbo Trainers, and Crocodile Banjo — all received the same texture fix. About 85–90 bytes of palette data were zeroed out across each model, fixing what appears to be a color bleed artifact on Kazooie's feathers.

### Brentilda Got a Makeover (ID 1337)

Brentilda's entire model was recompiled from scratch — her geometry, display list, and animation references were all rebuilt, making the model about 1KB smaller. Likely a rendering fix for the gossip-loving witch.

### Jinjonator Pedestals Fixed (ID 1349)

The stone Jinjo pedestals in the final boss fight were significantly reworked. The model shrank by 36% and the collision mesh was completely rebuilt. In v1.0, the collision data may have allowed unintended interactions during the Jinjonator activation sequence — v1.1 simplified it.

### Treasure Trove Cove Geometry Overhaul (9 models)

TTC received the biggest single change in the entire revision. The main opaque map model had over 150,000 bytes of changes — more than a third of the entire model was different. The Lighthouse structure was integrated directly into the map geometry rather than being a separate attachment. The transparent geometry, Crab Shell, Pirate Ship, Sand Castle, and Sharkfood Island sub-areas all received minor reference updates.

### Other Level Fixes

- **Mad Monster Mansion's Well**: Nearly half the model was rewritten, shrinking by 488 bytes
- **Rusty Bucket Bay's Engine Room**: Major geometry rework, nearly half the model changed, growing by 472 bytes
- **Click Clock Wood Winter**: Moderate changes to transparent geometry (1480 bytes differ)
- **Gruntilda's Lair Floor 5** and several other levels: Minor internal reference updates

### Portrait Models Updated (6 models)

The paintings in Mad Monster Mansion (Gruntilda, Blackeye, Tower, Tree & Moon, Tee-Hee, and the Seaman/Grublin/MumMum/Limbo portrait) all had a single internal reference updated. Freezeezy Peak's Christmas Tree had the same change. These were renumbered to match the v1.1 asset table reorganization.

### Map Setup Adjustments (6 maps)

- **Treasure Trove Cove** and **Freezeezy Peak**: One object placement added to each map, plus objects reordered within their spatial cells
- **Gobi's Valley** and **Intro Cutscene**: Objects reordered within cells (draw order / load priority change)
- **Click Clock Wood Fall**: 3 duplicate nodeprops removed from cube (2,0,2) — two were identical spawn triggers at the exact same position (2366,0,2931), plus a redundant ActorSpawn nearby. One new ActorSpawn was added to cube (0,0,0). Several actors in cube (0,0,0) had their flags updated.
- **Click Clock Wood Winter**: 23 cubes changed across the map. About 15 actors had a single flag bit enabled (`+0x10`, e.g. `0x2F80` → `0x2F90`) — likely a visibility or collision flag that was incorrectly off in v1.0. Three model props had their scale corrected (one went from 0.00 to 0.16, fixing an invisible object). One actor near Nabnut's house at (-764,5590,-2634) had a significant flag change (`0x2BA0` → `0x43B0`), suggesting a behavior or type correction.

### Every Quiz Question Rewritten (200 questions)

All 170 Furnace Fun quiz questions and all 30 Grunty questions were completely replaced. Even if the content is the same, not a single question from v1.0 is identical to v1.1. This is the largest content change in the revision.

### Cheato's Codes Highlighted (3 dialogs)

The three Cheato cheat code dialogs (BLUEEGGS, REDFEATHERS, GOLDFEATHERS) were reformatted to display the code words in a highlighted color, making them easier to read. The codes themselves didn't change.

### Font System Reworked (69+ glyphs)

The old monolithic FontSprite (ID 1769) was removed entirely and replaced with individually-sized glyph sprites. Some glyphs that were tiny placeholder stubs in v1.0 (152 bytes) were replaced with full rendered characters (up to 7000 bytes). Others were re-encoded more efficiently. The alpha mask sprite also grew to accommodate the new glyph dimensions.

### Debug Leftovers Cleaned Up (11 models)

Eleven 3D models used during development were stripped: media player icons (Eject, FastForward, Camera), audio debug markers (MusicSymbol, SFX), and credits role text (MAKE, TILK, KILL). These were never used by gameplay code.

### Removed Dialogs (13)

Six additional Cheato dialog variants were removed, along with unused dialog branches for Gloop, Gnawty, and Mutant-Snippet. One Conga dialog ("THIS CONGO'S TREE") had a typo fixed to "THIS CONGA'S TREE" in v1.1.

### New Final Battle Dialog (1)

One new Gruntilda line was added: "YOU JOGGED MY ARM, I'M IN A HUFF" for the final battle sequence.

### Speed Shoes Texture

The speed shoes (Turbo Trainers) had a star design on the side in v1.0/PAL. In v1.1 and JP, this was changed to a thunderbolt — matching how they appear in Banjo-Tooie. This is part of the texture data in the TurboTalonTrainers model (ID 871).

### Gameplay Bug Fixes (Code Changes)

These changes are in the game code, not the asset data. Since the decomp is based on v1.0, the port currently has v1.0 behavior. Fixes marked with `PORT_FIX` have been applied and are compiled in by default. Others are pending investigation.

Code fixes applied via `#ifdef PORT_FIX`:
- **Claw swipe before learning** — `src/core2/bs/player_spawn.c`: slide check moved before claw check
- **Grunty defeated flag timing** — `src/fight/chjinjonatorbase.c` + `src/fight/chfinalboss.c`: flag moved from egg-hit to fireball cutscene
- **Boggy race game over** — `src/core2/bs/player_spawn.c`: skip game over when `maSlalom_isActive()`, reload map instead
- **CCW Spring rock** — `src/CCW/ch/destruct_rock.c`: rock ignores collision callback in Spring
- **CCW flower softlock** — `src/CCW/ch/seasonal_npc.c`: egg hitbox disabled after `FILEPROG_E3_CCW_FLOWER_SPRING` is set
- **Pushing/bouncing Grunty** — `src/fight/chfinalboss.c`: collision passive handler returns early in phase 5+
- **Grunty fight Jinjo sound** — `src/fight/chbossjinjo.c`: powerup sound stopped immediately when Jinjo enters hit state
- **Termite Mound slopes** — `src/core2/ba/ba_recoil.c`: slope timer fills instantly in termite mound (v1.0 doubled the rate, v1.1 makes it immediate)
- **Yum-Yum crash** — `src/TTC/ch/clam.c`: cap dropped eggs at 8 and feathers at 5 per map visit to prevent spawn overflow crash

| Bug | v1.0 | v1.1 Fix | Versions Fixed |
|-----|------|----------|----------------|
| **Conga's name typo** | "THIS CONGO'S TREE" | "THIS CONGA'S TREE" | v1.1, PAL, JP (asset fix — dialog text) |
| **CCW Spring rock destructible** | Rock blocking Gnawty's house can be destroyed in Spring via egg sniping | Made indestructible in Spring | v1.1, JP (PAL still has the bug) |
| **CCW flower softlock** | Planting the flower multiple times in Spring causes a permanent softlock | Egg hitbox removed after first use | v1.1, JP |
| **Termite Mound slopes** | Banjo can stand on steep slopes briefly, allowing climbing without termite transformation | Instant slide-off on all termite mound slopes | v1.1, JP |
| **MMM Well OOB** | Player can escape through the top of the well | Collision added to well top | v1.1, PAL, JP |
| **RBB Engine Room pipe** | Loading zone allows seeing out-of-bounds in first-person | Black ceiling blocks OOB view | v1.1, PAL, JP |
| **Claw swipe before learning** | Can trigger claw swipe via slide+B in basic training before learning the move | Glitch patched | v1.1, JP |
| **GV Lobby floor gap** | Beak barge through floor collision gap in Gobi's Valley lobby | Gap sealed | v1.1, PAL, JP |
| **Boggy race game over** | Losing Boggy's race with 0 lives triggers game over | Teleports back to race start instead | v1.1, JP |
| **Game Over text formatting** | "YOU........." on a separate line from the rest of the sentence | Text reformatted to fit on one line | v1.1, PAL |
| **Grunty defeated flag timing** | Flag set when last egg hits Jinjonator — can die before cutscene | Flag set on first frame of fireball cutscene | v1.1, PAL, JP |
| **Grunty fight Jinjo sound** | Hitting Grunty with Jinjo before charge animation ends loops the sound forever | Sound stops on hit | v1.1, PAL, JP |
| **Pushing/bouncing Grunty** | Can push defeated Grunty off her platform with attack moves | Grunty made immovable after defeat | v1.1, PAL, JP |
| **Yum-Yum crash** | Unlimited eggs/feathers on ground can crash the game | Capped at 8 eggs / 5 feathers on ground | JP only |
| **Pause during death** | Pausing during death animation with 1 life causes premature game over | Can't pause during normal/pit death animations | JP only |

### PAL Demo Paths Redone

All attract mode demo input sequences were re-recorded for the PAL version to account for the 50Hz/25fps timing difference. The US demo data desyncs at PAL framerate. This matches our finding that PAL has different demo input data at the same IDs.

### Asset Table Reorganization

Beyond the content changes, v1.1 compacted the entire asset ID space. Level geometry and MIDI tracks that occupied IDs 5000+ in v1.0 were moved down to the 3000+ range. This means the same numeric ID can refer to completely different asset types between versions — for example, ID 3500 is a dialog in v1.0 but a MIDI track in v1.1.

---

## What Changed in PAL

The PAL version (NBKP) is built on top of v1.1. It includes every fix listed above and adds localization support.

### Multi-Language Dialogs

The biggest PAL change: every dialog, quiz question, and Grunty question uses a multi-language format. Per TCRF, the PAL version supports **3 languages: English, German, and French** — selected via a language menu on file select that reuses the Furnace Fun question UI with country flags (UK, Germany, France) and a 10-second timer.

Binary analysis of all 749 PAL dialog blobs confirms exactly 3 language blocks with no empty slots or room for additional languages. The `07` in the header is a format version identifier, not a language count. The full format:

```
[03]           PAL format marker
[07]           format version
[00]           padding
[u16 LE]       byte offset to French text block (from start of blob)
[u16 LE]       byte offset to German text block (from start of blob)
[English]      standard US dialog format (bottom_count, entries, top_count, entries)
[French]       same format, French translation
[German]       same format, German translation
```

The language offsets are **little-endian u16** — notably different from the rest of BK's big-endian data. Each language block uses the same `[count] [cmd slen text]...` structure as US dialogs, so a PAL dialog parser only needs to extract the correct language slice and then parse it identically to US format.

### Extra Font Characters

PAL includes 220 modified font glyphs (vs 69 in v1.1) plus 3 entirely new sprite entries. The additional glyphs cover accented characters needed for German and French text — é, ü, ö, ß, and similar.

### Cutscene Adjustments (2 maps)

Two cutscene setups were modified that v1.1 left untouched:

- **Grunty's Intro Cutscene**: 88 bytes of additional object data (+4–5 objects). Likely repositions text boxes or adjusts timing for the longer multi-language intro text.
- **Grunty's Flying Cutscene**: 6 bytes differ — three are small coordinate shifts (+4, +4, +8 units) consistent with adjusting object positions for the wider PAL display aspect ratio (5:4 vs NTSC 4:3). One prop reference was also changed.

Additionally, the Intro cutscene setup differs from both US versions — PAL applied its own changes on top of v1.1's prop reordering.

### Debug Models Kept

Unlike v1.1 which stripped the 11 debug models entirely, PAL keeps them at the same IDs but with modified content — possibly updated for PAL development tools.

---

## Japanese Version (NBKJ)

### Overview

The Japanese version (バンジョーとカズーイの大冒険, December 1998) is **based on v1.1**, not v1.0. It shares 1681 assets identical-at-same-ID with v1.1 (vs 1618 with v1.0), and contains all v1.1 bug fixes: the FinalBattleStoneJinjo collision rework, Brentilda recompilation, Kazooie texture patch, and all the same model/level/map fixes.

| | US v1.0 | US v1.1 | PAL | JP |
|---|---|---|---|---|
| Game code | NBKE | NBKE | NBKP | NBKJ |
| Non-empty assets | 3314 | 3044 | 3059 | 3065 |
| Max asset ID | 5570 | 3615 | 3616 | 3640 |
| Dialog format | US single-lang | US single-lang | 3-lang (EN/FR/DE) | US single-lang (Japanese text) |
| Debug models | Present | Removed | Present (modified) | Removed |
| Font system | FontSprite + stubs | Individual glyphs | Individual glyphs + accented | Individual glyphs + 13 kanji sheets |

### JP Uses v1.1 as Base

Every v1.1 fix is present in JP:
- **FinalBattleStoneJinjo** (ID 1349): Identical to v1.1 (reworked collision)
- **Brentilda** (ID 1337): Identical to v1.1 (recompiled model)
- **Kazooie texture** (IDs 845, 846): Identical to v1.1 (feather color fix)
- **Debug models 1777–1787**: Removed, same as v1.1
- **FontSprite 1769**: Missing, same as v1.1

### Japanese Text (US Dialog Format)

Unlike PAL which uses a multi-language format, JP dialogs use the **same `01 03 00` header as US** — just with Japanese text. This means Torch's existing DialogFactory parses JP dialogs without modification. The text content is fully translated to Japanese but the binary format is identical to US.

### Kanji Font Sheets (Mode 7 — JP Exclusive)

JP has a unique **mode 7** that doesn't exist in any other version: 13 large sprite sheets (IDs 3628–3640) containing Japanese font characters (kanji, hiragana, katakana). Each is a CI4 sprite at 190x190 pixels, ranging from 2KB to 41KB. These provide the character graphics needed for Japanese text rendering — a fundamentally different approach from the Western versions' individual glyph sprites.

| Mode | Type | JP IDs |
|------|------|--------|
| 7 | Japanese Font Sprites | 3628–3640 |

### Mode Layout

JP's mode boundaries are shifted slightly from v1.1 due to the extra assets:

| Mode | Type | JP IDs | vs v1.1 |
|------|------|--------|---------|
| 0 | Animation | 0–713 | Same |
| 1 | Model/Sprite | 721–1819 | Same |
| 2 | Map Setup | 1821–1973 | Same |
| 3 | Model/Sprite | 1978–2210 | Same |
| 4 | Dialog/Demo/Quiz/Sprite | 2211–3280 | +9 entries |
| 5 | Level Model | 3281–3451 | +9 offset |
| 6 | Midi | 3452–3624 | +9 offset |
| 7 | Japanese Font | 3628–3640 | JP exclusive |

### Content Comparison

| Category | Count | Notes |
|----------|-------|-------|
| Identical to US v1.0 (hash match) | 1992 | Animations, models, sprites, midi, level geometry |
| Same asset, minor patch | 144 | Same fixes as v1.1 + JP-specific tweaks |
| Font glyphs (rebuilt) | 222 | Same rebuild as v1.1 + JP-specific glyphs |
| JP-localized dialogs | 472 | Japanese text, US binary format |
| JP quiz questions | 170 | All new (same as v1.1 — rewritten from v1.0) |
| JP grunty questions | 30 | All new |
| Level model changes | 21 | Same 13 as v1.1 + 8 additional JP-specific level changes |
| JP kanji font sheets | 13 | Mode 7 exclusive |
| JP-only sprite | 1 | ID 1770 (adjacent to removed FontSprite 1769) |

### What's Unique to JP

1. **8 additional level model changes** beyond the 13 shared with v1.1 — the Lair in particular has more modified geometry (Floor1, Floor2, Floor3, Floor4_BGSEntrance, Floor5, Floor6_LavaRoom, Floor6_CoffinRoom, Floor7_MMMPuzzle). These may be Japanese text signage or navigation aid adjustments.

2. **Mode 7 kanji font sheets** — 13 sprite sheets for Japanese character rendering, a system that doesn't exist in other versions.

3. **ID 1770 sprite** — a JP-exclusive sprite at the slot right after the removed FontSprite (1769). Likely part of the Japanese font system.

4. **5 JP-unique dialogs** at IDs 3064–3071 that differ from their US counterparts despite being at the same ID — these are JP-specific dialog rewrites (Clanker, Tanktup, Mr. Vile, Choir Member, Gobi).

---

## Technical Reference

This section covers the raw asset table structure for port and mod developers.

### Asset Table Layout

All versions share offset `0x5E90`. The table is divided into mode regions:

| Mode | Type | v1.0 IDs | v1.1 IDs | PAL IDs | JP IDs |
|------|------|----------|----------|---------|--------|
| 0 | Animation | 0–713 | 0–713 | 0–713 | 0–713 |
| 1 | Model/Sprite | 721–1819 | 721–1819 | 721–1819 | 721–1819 |
| 2 | Map Setup | 1821–1973 | 1821–1973 | 1821–1973 | 1821–1973 |
| 3 | Model/Sprite | 1978–2210 | 1978–2210 | 1978–2210 | 1978–2210 |
| 4 | Dialog/Demo/Quiz/Sprite | 2211–3499 | 2211–3271 | 2211–3272 | 2211–3280 |
| 5 | Level Model | 5227–5397 | 3272–3442 | 3273–3443 | 3281–3451 |
| 6 | Midi | 5398–5570 | 3443–3615 | 3444–3616 | 3452–3624 |
| 7 | Japanese Font | — | — | — | 3628–3640 |

Modes 0–3 are identical across all versions. Mode 4 shrinks in v1.1/PAL/JP. Modes 5–6 relocate from the 5000s to the 3000s. Mode 7 is JP-exclusive (kanji font sheets). This creates **386 IDs that change asset type** between v1.0 and v1.1.

### Geo Reference Shift (+0x1AB)

Multiple models contain internal geo layout references that shifted by exactly +0x1AB between v1.0 and v1.1/PAL. This affects the portraits, Christmas Tree, and several level models. The shift is systematic — the referenced asset set moved as a block during the reorganization.

### PAL Dialog Format

| Type | US Header | PAL Header |
|------|-----------|------------|
| Dialog | `01 03 00` | `03 07 00` + 2 × LE u16 offsets, then EN/FR/DE blocks |
| Quiz Question | `01 01 02 05 00` | `03 01 02` (likely same offset table pattern) |
| Grunty Question | `01 03 00 05 00` | `03 03 00` (likely same offset table pattern) |

PAL dialog offsets are **little-endian u16** — the only known LE data in BK's otherwise fully big-endian format. Each language block after the offset table uses identical structure to US dialogs, so parsing a single language requires only slicing the blob at the correct offset.

### Impact on Port — Status

| # | Item | Status | Notes |
|---|------|--------|-------|
| 1 | **VER_SELECT IDs** | DONE | All 4 slots (v1.0/PAL/v1.1/JP) populated for all 8 non-stubbed VER_SELECT calls in `enums.h` and `snacker_ctl.c`. v1.1/PAL/JP all share the same relocated IDs. Remaining VER_SELECT calls are CRC checksums, DMA block sizes, and debug line numbers — all in stubbed code. |
| 2 | **Font sprite 1769** | DONE | Debug font overlay, not gameplay text. `func_802E4E54` in `render.c` stubbed with `return 0`. FontSprite 0x6E9 was a dev debug asset removed in v1.1/PAL/JP. |
| 3 | **Mode 4 type changes** | DONE | IDs 3072–3224 are dialogs in v1.0 but quiz questions in v1.1/PAL/JP. The symbol map handles this at the Torch level — Torch detects the actual data type regardless of the name prefix. |
| 4 | **PAL dialog parsing** | DONE | All 3 Torch factories (Dialog, QuizQuestion, GruntyQuestion) handle both US (`01 03 00`) and PAL (`03 07 00` / `03 01 02` / `03 03 00`) headers. PAL dialogs export all 3 language blocks (EN/FR/DE) to the o2r. The port importer reconstructs the original PAL blob format with LE u16 offset table so `dialogBin_get` handles language selection natively. |
| 5 | **Asset ID remapping** | DONE | `AssetVersionRemap.h` contains v1.0→v1.1 (934 entries), v1.0→PAL (934 entries, merged from v1.1 + PAL-specific), and v1.0→JP (794 entries) remap tables. `ResourceHelpers.cpp` auto-detects o2r version at boot by manifest entry count and injects the appropriate aliases. Zero cost when using a v1.0 o2r. |
| 6 | **JP text rendering** | DEFERRED | JP has 13 kanji sprite sheets in mode 7. The decomp's text system doesn't handle Japanese characters. Game would load and play with JP o2r, just with broken text in dialog boxes. Not blocking. |
| 7 | **Multi-language support** | DONE | PAL o2r exports all 3 language blocks per dialog. The importer reconstructs the PAL blob format preserving the offset table. The decomp's native `dialogBin_get` and `code94620_func_8031B5B0` handle language selection at runtime. `func_8031B5BC` returns `ResourceMgr_GetDialogLanguageCount()`. Language is selectable via Settings > Languages > Dialog Language combobox (greyed out for US/JP o2rs, enabled for PAL). CVar persists across sessions. |
| 8 | **Runtime VER_SELECT** | NOT NEEDED | The remap table handles all asset ID translation. Only would be needed for PAL 50Hz timing differences, which aren't relevant to the port. |
| 9 | **Boot sequence** | DONE | `getDefaultBootMap()` in `init.c` reads the `gSettings.BootSequence` CVar via `port_getBootSequence()`. Options: Default/Authentic (Rareware logo intro), File Select (skip intros). |
