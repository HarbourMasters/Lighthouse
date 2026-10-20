# Making a Dialog Mod

The [language pack guide](LANGUAGE%20PACKS.md) covers translating dialog. This guide covers the mechanics behind dialog assets: portraits, choices, text substitution, inline formatting, and gameplay triggers.

---

## 1. Anatomy of a dialog

A dialog asset has two lists of boxes, `Bottom` and `Top`, one for each screen position. Each row is a `[code, "text"]` pair:

```yaml
dialog/ASSET_A0D_BLUBBER_COMPLETE:
  Bottom:
    - [0x87, "ME TREASURE! THANK YE ME HEARTIES, TAKE THIS REWARD!"]
    - [0x7, '\x01']
    - [0x87, "I'M OFF TER SPEND, SPEND, SPEND!"]
    - [0x4, ""]
  Top:
    - [0x4, ""]
```

There are **two separate code layers**:

- the **box code** - the leading byte of each row, interpreted by the dialog sequencer ([`src/core2/gc/dialog.c`](../../src/core2/gc/dialog.c));
- **inline codes** - bytes *inside* the quoted text, interpreted by the text renderer ([`src/core2/font/print.c`](../../src/core2/font/print.c)).

`0x04` as a box code closes the box; `0x04` inside a string is nothing.

The asset ids and names in this guide are the same on every base.

---

## 2. Box codes

### Portraits: `0x80` and up

A box code of `0x80` or higher identifies a text box spoken by portrait `code - 0x80`. The portrait determines the presentation: talk sprite, voice, and mouth animation. There is no separate "play voice" code. The full cast is the `GcZoomboxSprite` table in [`include/core2/gc/zoombox.h`](../../include/core2/gc/zoombox.h) (a dialog byte maps to that table at `code - 0x80 + 0x0C`). Some regulars:

| Code | Speaker | | Code | Speaker |
|---|---|---|---|---|
| `0x80` | Banjo | | `0x87` | Blubber |
| `0x81`/`0x82` | Kazooie | | `0xB5` | Gruntilda |
| `0x83` | Bottles | | `0xCB` | Brentilda |

Valid codes run `0x80`-`0xDE`; the retail game tops out at `0xDB`. Consecutive boxes with the *same* portrait code are grouped into one on-screen box as successive lines (up to 8), so a new code is also what forces a new box.

### Commands: `0x01`-`0x09`

Codes below `0x20` are commands, not text. The game implements nine:

| Code | Name | What it does |
|---|---|---|
| `0x01` | **Choice** | A/B prompt. The result (A = 1, B = 0) is handed to the actor's completion callback, which decides what happens - the dialog itself doesn't branch. |
| `0x02` | Yield, keep alive | Minimize this box but let it survive into the next dialog (used to chain conversations). |
| `0x03` | Choice, closing | Like `0x01`, but both boxes close after the answer. |
| `0x04` | **Close** | Shut this box and hand over to the other list. The standard terminator - nearly every `Bottom` and `Top` ends with one; the exceptions end on a choice or a `0x02` yield into the next conversation. |
| `0x05` | Minimize, yield | Implemented but unused by the retail game. |
| `0x06` | **Hand over** | The back-and-forth marker: minimize, pre-load the next speaker's portrait, and switch to the other box. |
| `0x07` | **Trigger** | Fire a gameplay event - see [#4](#4-scripting-gameplay-the-trigger-box). |
| `0x08` | Text variant | One replacement candidate for a `~` in the preceding text box - see [#3](#3-live-substitution). |
| `0x09` | Number slot | Replace the `~` in the preceding text box with a live number - see [#3](#3-live-substitution). |

Codes `0x0A`-`0x1F` fall through to garbage - don't use them. Box counts are recomputed on import, so you can add and remove rows.

---

## 3. Live substitution

A `~` in a text box is a placeholder, filled in when the box is shown. What fills it depends on the rows that follow:

- **`0x08` rows** offer fixed alternatives; a callback from the actor showing the dialog picks one. Brentilda's gossip works this way - the right answer is baked into the save:

  ```yaml
  - [0xcb, "GRUNTY BRUSHES HER ROTTEN TEETH WITH ~ FLAVORED TOOTHPASTE!"]
  - [0x8,  "TUNA ICE CREAM"]
  - [0x8,  "SALTED SLUG"]
  - [0x8,  "MOULDY CHEESE"]
  - [0x4,  ""]
  ```

- **A `0x09` row** (empty string) makes the game print a number instead - the actor's callback supplies the value.

The actor's code selects the variant, so you can reword the candidates and surrounding text. Keep the number of `0x08` rows unchanged, though: the actor expects a specific number of candidates.

---

## 4. Scripting gameplay: the trigger box

A trigger box never appears on screen. When the conversation gets to one, the dialog system calls the **trigger callback** the actor registered when it opened the dialog, and passes along the box's first text byte as an **event id** ([dialog.c:443](../../src/core2/gc/dialog.c#L443)). Then it skips past the box and the conversation carries on.

Blubber end to end:

1. His dialog `ASSET_A0D_BLUBBER_COMPLETE` (shown in [#1](#1-anatomy-of-a-dialog)) carries `[0x7, '\x01']` between "TAKE THIS REWARD!" and "I'M OFF TER SPEND...".
2. When Blubber opens it, he passes two callbacks ([blubber.c:96](../../src/TTC/ch/blubber.c#L96)) - one for completion, one for triggers.
3. The trigger callback ([blubber.c:77](../../src/TTC/ch/blubber.c#L77)) calls `jiggy_spawn(JIGGY_14_TTC_BLUBBER, ...)` and puffs steam at the spawn point.

So the jiggy pops **mid-sentence**. Move the `0x07` row and the spawn moves with it; delete it and the jiggy never appears; add a second and the spawn fires twice. (The variant dialog for when you've already collected the jiggy, `ASSET_A2A`, has no trigger box.)

The **event id byte** is the actor's to interpret. Blubber ignores it; any trigger spawns his jiggy. Bottles' molehills switch on it ([mole.c:201](../../src/core2/ch/mole.c#L201)): id `\x05` grants 50 eggs mid-lesson, `\x06` grants red feathers, `\x08` refills health, `\x01`-`\x04` move the tutorial camera.

Two rules:

- **A trigger is a request, not an opcode.** It does whatever the showing actor's callback does. There is no generic "spawn object" code, and a `0x07` box in a dialog whose actor registered no trigger callback does nothing.
- **Skipping doesn't lose events.** If the player skips out of a dialog, every not-yet-reached trigger box fires anyway ([dialog.c:281](../../src/core2/gc/dialog.c#L281)).

---

## 5. Inline format codes

Inside the text, any byte that isn't a glyph in the current font falls into a command switch, and `0xFD` explicitly escapes the *next* byte into it. The useful ones:

| Sequence | Effect |
|---|---|
| `\xFDh` ... `\xFDl` | **Shaky text** on ... off (per-glyph wobble) |
| `\xFDf` | Toggle dialog <-> bold font |
| `\xFDb` | Toggle a black panel behind the text |
| `\xFDp` | Toggle monospaced spacing |
| `\xFDj` ... `\xFDe` | Enter/leave the extended (JP) glyph sheet, where bytes are raw glyph indices |

Type them in **single-quoted** YAML (`'I FEEL \xFDhALL\xFDl FUNNY'`) - the importer decodes `\xNN` escapes there, while double quotes hand them to the YAML parser instead (see the [language pack guide's encoding notes](LANGUAGE%20PACKS.md#how-the-text-is-encoded)). Inside single quotes an apostrophe is written twice: Blubber's line becomes `'I''M OFF TER SPEND, SPEND, SPEND!'`. Pauses, scroll speed, and line wrapping are *not* inline codes: punctuation pauses automatically, the player controls speed, and wrapping is automatic at 24 printable characters.

Because the renderer treats non-glyph bytes as commands, a **bare lowercase** `b d e f h j l p q v` can execute a command without any `\xFD` prefix. Stock text is all-caps, so vanilla never trips this, but your edits can. Stick to uppercase (or a font that actually has lowercase glyphs, which makes those bytes ordinary letters again). The `\xFD`-prefixed form is always safe. JP is the exception: its text exports as raw glyph indices inside `\xFDj` ... escapes, so the letters there are glyph numbers and this advice doesn't apply.

---

## 6. Quiz and Grunty questions

Furnace Fun's quiz (`quizq/`) and the final boss's round (`gruntyq/`) are their own asset types, but they share this text encoding. The YAML splits a question into its `Text` lines and its answer `Options`:

```yaml
quizq/ASSET_1213_FF_QUIZ_QUESTION:
  Text:
    - [0x80, "YOU FOUND ENOUGH, YOU KNOW THE SCORE,"]
    - [0x80, "HOW MANY NOTES FOR THE FIRST NOTE DOOR?"]
  Options:
    - [0x81, 0xfd, 0x6c, "50"]
    - [0x82, 0xfd, 0x6c, "100"]
    - [0x83, 0xfd, 0x6c, "75"]
```

An option is a 4-element row whose two middle bytes must be kept. PAL's quiz options have no such bytes and export as `[0x81, "50"]`. Any other row makes the importer log an error and leave the question out of `bk.o2r`, but the import still finishes, so check the log. Every question carries three options, numbered `0x81`, `0x82`, `0x83`.

### Which option is the correct one

The YAML doesn't say. The port gives each question type a number of *candidate answers* ([`DialogFactory.cpp:196`](../../src/port/Resource/Importers/DialogFactory.cpp#L196) and `:218`); the rest are decoys. The question manager picks the correct one, then fills the other two slots at random from what's left ([`questionmanager.c:306`](../../src/core2/quiz/questionmanager.c#L306)).

**Quiz questions have one candidate, so `0x81` is always the correct answer.** `0x82` and `0x83` are the decoy pool and are always wrong. All three still appear on screen every time; the shuffle only decides which line each one lands on.

**Grunty questions have three candidates, so any of the three can be correct.** The save decides which answer is correct. The first time Brentilda spawns in the lair, she writes a seed to the save file. Every question's answer derives from that seed, so the answers stay fixed for that save. Brentilda's gossip ([#3](#3-live-substitution)) reads out that same value. Nothing in the asset marks the true one, so **a Grunty question's three options have to stay interchangeable**. Reword them freely, but don't write a set where one answer is obviously the real one.

---

## 7. Build and test

The workflow is the same as for other asset types: edit the YAML, run `torch modding import o2r`, stage the changed `assets/dialog/...` entries under `alt/` (`alt/assets/dialog/...`), package them with `torch pack`, and put the resulting mod in `mods/`. On PAL the export puts a `French` and a `German` map beside the English, with the same keys, and the import writes all three back. Give an asset both or neither: one with neither shows its English in every language, and one with only one of them is left out with an error. Translating this text rather than rewriting it is the [language pack guide's](LANGUAGE%20PACKS.md#quiz-and-grunty-questions) job.

To test it: move Blubber's `[0x7, '\x01']` row *above* his first line, deliver his gold, and the jiggy pops before he says a word.
