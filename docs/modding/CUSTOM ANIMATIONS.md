# Making an Animation Mod

The *Fast Swim* Banjo-Tooie backport plays modified animation data when both A and B are held while swimming. It overlays Banjo's leg kicks onto Kazooie's wing stroke. This guide rebuilds the kick half.

We'll work on the keyframes directly, as YAML. If you'd rather pose a rig, the Fast64 BK64 plugin reads an animation onto an armature and writes one back out - see the [model guide](CUSTOM%20MODELS.md#3-building-a-model). The bone id rules in [#2](#2-the-keyframe-format) hold either way.

---

## 1. Export the animations to YAML

```
torch modding export <baserom.z64> -s <lighthouse> -d <workdir>
```

This decodes every asset into an editable form under `<workdir>/src/assets/assets/` and writes a `modding.yml` manifest listing them (leave `dialog_pack` **off**). Each animation comes out as its own YAML file, and we only need two of them: the slow swim kick (`ASSET_71_BSSWIM_DIVE_SLOW`) and the underwater stroke (`ASSET_3F_BSSWIM_DIVE_MOVE`). Sections 2-4 work on the kick by itself, and section 5 combines it with the stroke.

```
assets/anim/ASSET_3F_BSSWIM_DIVE_MOVE.yaml
assets/anim/ASSET_71_BSSWIM_DIVE_SLOW.yaml
```

Delete the YAML files you aren't going to change, and prune `modding.yml` to match. The kick on its own only needs `ASSET_71_BSSWIM_DIVE_SLOW.yaml`, but hang on to the stroke as well if you plan to follow section 5.

---

## 2. The keyframe format

An animation YAML is a frame range plus a list of **elements** - one per (bone, channel) pair. The top of the paddle kick:

```yaml
anim/ASSET_71_BSSWIM_DIVE_SLOW:
  StartFrame: 0
  EndFrame: 40
  Elements:
    - BoneIndex: 1
      TransformType: 0
      Data:
        - [1, 1, 0, -588]
        - [1, 1, 40, -588]
    ...
    - BoneIndex: 20
      TransformType: 1
      Data:
        - [1, 1, 0, 630]
        - [1, 1, 5, -870]
        - [1, 1, 12, -3813]
        - [1, 1, 25, -5715]
        - [1, 1, 35, -90]
        - [1, 1, 40, 630]
```

**`BoneIndex`** is a bone id in the target model's skeleton (a bone appears once per channel it animates). **`TransformType`** picks the channel:

| Type | Channel | Value means |
|---|---|---|
| 0 / 1 / 2 | rotation X / Y / Z | `value / 64` = degrees |
| 3 / 4 / 5 | scale X / Y / Z | `value / 64` = factor (so `64` = 1.0) |
| 6 / 7 / 8 | translation X / Y / Z | `value / 64` x the model's animation scale = world units |

Each row in `Data` is one keyframe, written `[flag1, flag2, frame, value]`. The frame says when the value applies, counted within the `StartFrame`-`EndFrame` range, and the two flags control how the animation eases into it. Almost every exported key uses `1, 1`, and it's best to leave them that way unless you're deliberately changing the interpolation.

Read the bone-20 element above with the table: it's rotation Y, swinging from `630` down to `-5715` and back across the 40-frame cycle. The playback *speed* isn't in this file. The game maps the frame range onto however long the action lasts.

Three rules the engine doesn't reliably enforce:

- **Keep one bone's elements contiguous.** The animation player walks the list accumulating channels and applies them when the bone id *changes*. Interleaving bones (6, 20, 6) silently mangles the result. Export order is always grouped; preserve it when adding elements.
- **Bone id 0 is reserved, and ids stop at `0x6C`.** Id 0 is skipped ([`animationfile.c:47`](../../src/core2/anim/animationfile.c#L47)). At the other end the transform list is a fixed `0x6D` entries ([`bonetransformlist.c:58`](../../src/core2/anim/bonetransformlist.c#L58)) and nothing checks an id against it.
- **Don't retype ids you don't know.** Bone ids come from the model being animated. You learn them from code that already uses them, or by nudging one channel at a time. For Banjo's skeleton, [`FastSwim.cpp`](../../src/port/Enhancements/Backports/FastSwim.cpp) documents the leg chains outright: **right leg `6, 7, 10, 12, 14`, left leg `20, 21, 24, 26, 28`**, with 14 and 28 the feet. And ids are per-model. The same numbers mean different limbs on a different skeleton, which is why a straight model swap T-poses; see the Jinjo swap in the [model guide](CUSTOM%20MODELS.md#2-wiring-a-model-into-the-game).

---

## 3. The edit: a harder kick

The thighs are the top of each leg chain - bones **6** (right) and **20** (left) - and the kick lives in their rotation-Y channels. To make the kick harder, multiply every value in those two `TransformType: 1` elements by 1.5 and round to whole numbers, leaving the frames and flags alone. Here's bone 20 after the change:

```yaml
    - BoneIndex: 20
      TransformType: 1
      Data:
        - [1, 1, 0, 945]      # was 630
        - [1, 1, 5, -1305]    # was -870
        - [1, 1, 12, -5720]   # was -3813
        - [1, 1, 25, -8573]   # was -5715
        - [1, 1, 35, -135]    # was -90
        - [1, 1, 40, 945]     # was 630
```

Do the same to bone 6's rotation-Y element.

---

## 4. Build the mod

Re-encode the archive with your edits:

```
torch modding import o2r <baserom.z64> -s <lighthouse> -d <workdir>
```

The importer re-encodes everything listed in `modding.yml` and fills in the rest from the ROM, so what you get in `<workdir>/bk.o2r` is a complete game archive rather than a mod. Your mod only needs the entries you actually changed:

1. An `.o2r` is just a ZIP, so any archive tool can open it. Extract `assets/anim/ASSET_71_BSSWIM_DIVE_SLOW` into an empty staging folder, putting it under `alt/` and keeping its folder structure: `<staging-folder>/alt/assets/anim/ASSET_71_BSSWIM_DIVE_SLOW`.
2. Pack the staging folder with Torch:

   ```
   torch pack <staging-folder> mymod.o2r o2r
   ```

Drop `mymod.o2r` into Lighthouse's `mods/` folder. Your `alt/` entry replaces the original while **Alternate Assets** is on. The [texture pack guide](TEXTURE%20PACKS.md#4-install-it) covers load order, for when two mods change the same thing.

Now test it: find water, dive, and paddle with A - the exaggerated kick is Banjo's ordinary slow swim.

---

## 5. Combined animation sets

Underwater movement is a *pair* of animations on a single player skeleton. Banjo's paddle kick works the leg bones while Kazooie's wings are static, and Kazooie's wing stroke works the wing bones while Banjo's legs are static. So the two combine without overlap. See [`FastSwim.cpp`](../../src/port/Enhancements/Backports/FastSwim.cpp) for the combined animation.

The enhancement does the combining in code, each frame, at independent speeds. But you can also build the combination as **pure data** - this is what the second YAML from step 1 is for. Both animations drive the same skeleton, and the wing stroke already carries a (near-static) element for every leg channel the kick animates - eighteen elements across the ten leg bones. In `ASSET_3F_BSSWIM_DIVE_MOVE.yaml`, find each leg element with the same `BoneIndex` and `TransformType` as one of the kick's, and replace its `Data` rows with the kick's, doubling every frame number but keeping the values and flags. Doubling the frames stretches the kick's 0-40 cycle across the stroke's 0-80.

Done this way, the kick is locked to the stroke, one kick for every stroke, whereas the enhancement plays the kick at its own speed. If you'd rather have two kicks per stroke, keep the kick's original frame numbers and add a second copy of its keys 40 frames later, so that the two copies meet at frame 40 and the last key lands on 80. Either way, you won't actually swim any faster - the speed boost comes from code, not from the animation.
