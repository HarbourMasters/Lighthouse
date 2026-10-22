# Making an Audio Mod

The game's audio comes in two kinds and they mod very differently.

A **sound effect** is one file. Drop an mp3 in next to it, run the import, done.

**Music** is a MIDI, and dropping one in is only half the job. Banjo plays a sequence differently from anything else that reads MIDI: the program numbers mean its own instruments, and *the map decides which of your channels are audible*. A channel the map leaves off stays silent, and nothing reports it.

---

# Part one: sound effects

## 1. Find the sound

Sounds are in `bk.o2r` at `assets/sfx/<id>_<NAME>`, named by the id the game plays them with:

```
assets/sfx/000_BLOOP
assets/sfx/0C2_GRUBLIN_EGH
assets/sfx/3FA_HONEYCOMB_TALKING
```

**Check how widely a sound is used first.** `002_CLAW_SWIPE` is named all over the decomp, and replacing it changes every one of those sounds. `0C2_GRUBLIN_EGH` is named in four files (grublin, grublinhood, gruntling, seamangrublin). A sound the game plays by number, or from a path or a table, isn't named anywhere.

To find the callers: `grep -rn SFX_C2_ src/`.

## 2. Export

```
torch modding export <baserom.z64> -s <lighthouse> -d <workdir>
```

Each sound comes out as three files:

| File | What it is |
|---|---|
| `0C2_GRUBLIN_EGH.yaml` | everything you might change: envelope, pitch, loop, chain |
| `0C2_GRUBLIN_EGH.wav` | the sound decoded, to listen to or edit |
| `0C2_GRUBLIN_EGH.bin` | the encoded sample: codebook, loop state, ADPCM |

Only assets listed in `modding.yml` are re-encoded on import; everything else is read straight from the ROM. The sounds are one `modding.yml` entry, `assets/sfx_bank`, and its import reads every sound's files, so keep all of `assets/sfx/` and change only the sounds you're replacing. Prune the rest of `modding.yml` to what you edit.

## 3. Drop your audio in

Copy your file next to the YAML and name it in the `Sample` key:

```yaml
Sample: sm64-mario-pain.mp3
```

`Sample: raw` (the default) rebuilds from the `.bin` byte for byte. Anything else is read as a filename sitting beside the YAML.

**Supported:** `.wav`, `.mp3`, `.flac`

**Rate, channels and length do not matter.** Whatever you hand it is mixed to mono, resampled to the slot's rate, and the envelope is fitted to its length.

**Leading and trailing silence are trimmed**, because the game only gives a sound a fixed amount of time to play, and the log tells you how much was cut. Set `TrimSilence: false` if you want to keep it. Looped sounds are never trimmed, since their loop points are counted in samples.

## 4. The envelope

A voice plays for as long as its **envelope**, not for as long as its sample. Hand the slot a longer sound and the game stops partway through; hand it a shorter one and the voice holds after the audio ends. In the vanilla banks the two match to within a millisecond.

**Torch fits it for you** when `Sample` names an audio file, and says so at `logging: INFO` (set it in your ROM's entry in the repo's `config.yml`, or in a copy passed as `-s`; see the [language pack guide](LANGUAGE%20PACKS.md#1-export-the-text-to-editable-yaml)):

```
Soundfont: sfx/0C2_GRUBLIN_EGH envelope fitted to the new sample -- DecayTime 231811 -> 311493 (0.315s)
```

`AttackTime` and `ReleaseTime` are left alone, so the shape of the attack and the fade-out survive; decay takes up the slack. Sounds left on `Sample: raw` are never touched. Set `FitEnvelope: false` to use your numbers unchanged.

### Working it out yourself

You need this if you are hand-building a sample ([#8](#8-building-the-sample-by-hand)) or overriding the fit. Envelope times are stored *unpitched*. The game divides them by the slot's pitch ratio when it plays, the same ratio it applies to the sample, so the two stay in step.

```
total microseconds = clip_seconds x 1,000,000 x pitch
                     where pitch = 2 ^ ((KeyBase - 60) / 12)

DecayTime = total - AttackTime - ReleaseTime
```

Take the clip from [#7](#7-build-and-pack), which is 6939 samples (0.5607 s) long, in a slot with `KeyBase` 50. The slot plays at 22050 Hz x pitch, so `clip_seconds x pitch` is `samples / 22050`, which is how Torch works it out:

```
total = 6939 x 1,000,000 / 22050 = 314,693 us
DecayTime = 314,693 - 0 - 3,200 = 311,493
```

| KeyBase | pitch | | KeyBase | pitch |
|---|---|---|---|---|
| 60 | x1.000 | | 48 | x0.500 |
| 55 | x0.749 | | 45 | x0.420 |
| 50 | x0.561 | | 42 | x0.354 |
| | | | 36 | x0.250 |

## 5. Pitch

Export a sound, open the `.wav`, and it sounds like the game; but its sample rate isn't 22050 Hz. `0C2_GRUBLIN_EGH` comes out at 12375 Hz, `000_BLOOP` at 7796.

The samples in the ROM are stored *high-pitched* and the game brings them down through `KeyBase`: 60 means play at the stored pitch, and every 12 below that is an octave down.

Export and import both use the rate the game plays at, so **leave `KeyBase` alone**. Setting it to 60 to "fix" the pitch appears to work, but `KeyBase` is read elsewhere in the audio path.

## 6. The other fields

The export names these fields by what they do for a sound effect:

| Field | What it does |
|---|---|
| `KeyBase`, `Detune` | pitch; see [#5](#5-pitch) |
| `ChainNext` | plays another sound after this one, by id; 0 for none |
| `ChainDelayFrames` | how long to wait before that one |
| `VolumeGroup` | which of 64 volume groups the sound belongs to |
| `ReverbSend` | reverb amount, 0-15 |
| `SamplePan`, `SampleVolume` | as they sound |

If `ChainNext` isn't 0, the sound is one link of a chain, and each link starts its own delay after the trigger. Replacing a single link on its own will sound wrong.

These meanings are sound-effect only. Under `assets/instruments/` the same fields are key and velocity ranges.

## 7. Build and pack

```
torch modding import o2r <baserom.z64> -s <lighthouse> -d <workdir>
```

At `logging: INFO` the log tells you what it did:

```
Soundfont: sfx/0C2_GRUBLIN_EGH trimmed 0.254s of silence from the start and 0.047s from the end
Soundfont: encoded sfx/0C2_GRUBLIN_EGH from sm64-mario-pain.mp3 -- 6939 samples, 31.6 dB
Soundfont: sfx/0C2_GRUBLIN_EGH envelope fitted to the new sample -- DecayTime 231811 -> 311493 (0.315s)
```

**The dB figure is encode quality**, measured against the same decoder the game uses. Above roughly 20 dB is fine. Single digits mean something is wrong with the input: near-silence, or a clip that is mostly noise.

Then pull your sound's entry out of the built archive into a staging folder under `alt/`, and pack it alone. That's `assets/sfx/0C2_GRUBLIN_EGH`, staged as `alt/assets/sfx/0C2_GRUBLIN_EGH`, not `assets/sfx_bank`, which comes out unchanged:

```
torch pack <staging-folder> mymod.o2r o2r
```

Drop it in `mods/`.

## 8. Building the sample by hand

`Sample: raw` reads the `.bin` verbatim, so one from the SDK tools or your own encoder works too. Little-endian throughout:

```
u32  order              prediction order; 2 in every vanilla sound
u32  npredictors        how many codebook entries follow
u32  bookCount          = order x npredictors x 8
s16  book[bookCount]    the codebook, Q11 (2048 = 1.0)
u32  stateCount         16 for a looped sound, 0 otherwise
s16  state[stateCount]  decoder state at the loop point
u32  dataSize
u8   data[dataSize]     the ADPCM frames
```

- **`dataSize` must be a whole number of nine-byte frames.** Pad with silence rather than leaving a partial frame.
- **The codebook has to suit your sample.** Reusing the one from the sound you are replacing predicts your audio badly. It spends the 4-bit residual range on error, which is audible as roughness.

---

# Part two: music

## 9. Find the track

Music is in `bk.o2r` at `assets/comusic/COMUSIC_<id>_<NAME>`:

```
assets/comusic/COMUSIC_2_MM
assets/comusic/COMUSIC_22_MMM
assets/comusic/COMUSIC_5_TTC_VACATION_VERSION
```

Export gives you a standard MIDI file per track:

```
torch modding export <baserom.z64> -s <lighthouse> -d <workdir>
```

Format 1, one chunk per channel, at the sequence's own resolution. Open it in Sekaiju, a DAW, or any MIDI player.

## 10. Channels are zones

A world plays *one* sequence, and where you stand decides which channels of it are audible. Walk from the Mumbo's Mountain entrance to Conga's Tree and the game doesn't change track, it mutes some channels and unmutes others.

Here's Mumbo's Mountain, from Riposte's annotated export:

| Where you are | Channels heard |
|---|---|
| Main area (spawn, most of the map) | 0, 1, 2, 3, 4, 5, 12 |
| Outside Ticker's Tower | main, plus 8 and 14 |
| Conga's tree | 6, 7, 10, 11, 12 |
| Outside Mumbo's hut | 6, 7, 12, 13, 15 |
| Underwater | 9 |

Rare wrote channels 0-5 as a self-contained arrangement (drumkit, marimba, bassoon, trombone, bass, clarinet). The rest were reserved for the zone variations: taiko and shakers for Conga and Mumbo areas, snare rolls and "hups" for Ticker's Tower, a harp for underwater. Channel 12 (bird calls) is in every surface zone.

**Put your arrangement on the channels the map's main zone plays**, and use the others only for material meant for those specific places. Where a map defines no zones, all sixteen channels play all the time.

## 11. Instruments

A program change selects one of Banjo's own 85 instruments. **These are not General MIDI numbers.** A file written against a GM soundfont will point most channels at the wrong instrument. Any program above 85 is skipped by the sequence player, so those channels go silent.

Here are a few of them in `bk.o2r`:

```
assets/instruments/program1_marimba_keys0-76
assets/instruments/program12_harmonica_keys55-66
assets/instruments/program14_tuba
```

The number is the one you type in a sequencer. The key range appears when an instrument is multisampled and says which notes that sample covers. Instruments are shared across the whole soundtrack, so replacing one changes every track that uses it. They are one entry too, `assets/instrument_bank`, so keep all of `assets/instruments/` the same way.

## 12. Loops

The game loops per track, and there are two ways to say where.

**Markers**: put `loopStart` and `loopEnd` in the track. Sekaiju loops playback between them. Torch exports vanilla tracks this way.

**Or the file name**: `... LP 384.mid` loops back to tick 384. Files named for N64MidiTool work unchanged. Markers win if both are present. Renaming a track's file means changing its value in `modding.yml` to match, or the import can't find it.

A track with neither plays once and stops.

## 13. Volume

Each track has its own master volume, out of 32767, and most sit well below it. Mumbo's Mountain plays at 20000, about 4 dB down.

Set volume with a marker:

```
volume 32767
```

## 14. Controllers

Only some do anything:

| CC | Effect |
|---|---|
| 7 | channel volume |
| 10 | pan |
| 16 | priority |
| 64 | sustain |
| 91 | reverb send |

**CC 11 (expression) is ignored.** Dynamics written in expression won't be heard. Use CC 7 or note velocity.

Two ranges do something other than make sound:

- **CC 106-119 signal the game**, not the synth. The organ puzzle in Mad Monster Mansion waits on one. Sekaiju's auto-repeat feature uses CC 111, which lands squarely in this range, so don't use it.
- **CC 126 and 127 mute and unmute a channel.** In General MIDI these are Mono Mode and Poly Mode; here they will silence part of your track.

Torch warns about both on import. Anything else is ignored harmlessly.

## 15. Voices and file format

The synth has **24 voices**. Loud comes from density rather than level: a thin arrangement pinned to velocity 127 still sounds quieter than a full one at velocity 90.

Resolution is unconstrained. Save as **SMF format 0 or 1**; format 2 isn't supported, and neither is SMPTE timing.

## 16. Build and pack

Same as sound effects, with the track's entry, such as `assets/comusic/COMUSIC_2_MM`, staged under `alt/`:

```
torch modding import o2r <baserom.z64> -s <lighthouse> -d <workdir>
torch pack <staging-folder> mymod.o2r o2r
```

At `logging: INFO` the log reports what it read:

```
Music: looping 16 track(s) back to tick 384 from the file name
Music: built a sequence from MIDI -- 16 track(s), division 96, 30809 bytes
```

---

*The instrument names come from the `.ins` file Riposte uses when scoring for the game; the decomp names music tracks but not instruments.*
