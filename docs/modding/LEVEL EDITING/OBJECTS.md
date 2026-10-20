# Objects

Scenery, collectibles and lights - the parts of a setup with no behavior of their own.

## Scenery and collectibles

Three record kinds share the objects section, told apart by two flags:

| Kind | What | What you can set |
|---|---|---|
| **Model prop** | static scenery | asset id (`Model:` in the YAML; the setup file holds it minus `0x2D1`), position, yaw and roll (in 2-degree steps), scale (x100 = 1.0, so 2.55 is the ceiling) |
| **Sprite prop** | notes, eggs, feathers, grass tufts | asset id (`Sprite:` in the YAML; the setup file holds it minus `0x572`), position, scale, mirror flag, an animation phase, and three small RGB-subtract values (0-7 each) that tint the sprite |
| **Actor slot** | a spawned actor's runtime record | position and flags; the marker pointer is filled at runtime |

Model props are pure decoration with collision - no behavior. Anything that *does* something is an actor, and actors are placed by nodes, not props. See [Actors](ACTORS.md).

**One model, thirty props.** Freezeezy Peak places the decorative icicles, `Model: 0x326`, thirty times, and no two copies are alike: each has its own scale, from 44 to 131 (0.44 to 1.31 times the model's built size), its own yaw, and some are rolled off vertical.

The sprite prop's animation value is a **phase**, not a starting frame: it offsets one sprite's animation against another's, so identical sprites placed together don't animate in step. It counts in 32nds of a cycle, so 16 puts one sprite half a cycle behind its neighbor. It also picks the mirroring and the direction the cycle runs.

## Point lights

The lighting section is a list of point lights: position, an inner and outer fade radius, and an RGB color. They tint **actors** that come near them - the glow Banjo picks up walking through one. They don't light the map itself, and models load no lights of their own.

Only some actors take light at all: the ones Clanker's Cavern, Rusty Bucket Bay and the intro cutscenes register that way. A light reaches an actor inside its outer radius, and at most 16 reach one actor at a time. An actor that takes light but has none in range draws **black**.

In the YAML each light is a `Lights` row: `Position`, `Fade` (inner, then outer) and `Color`.

## Scenery that moves

To make a model prop move, place one of these actor nodes near it. At load, each one takes over the nearest model prop within 500 units; nothing on the prop itself changes.

| Actor node | Effect |
|---|---|
| `0x37` water bobber | the prop floats and bobs on water |
| `0x13` sinking bobber | the prop sinks while Banjo stands on it |
| `0x38` Tumblar movement | Tumblar, in `MMM_TUMBLARS_SHED` only |
| `0xF9`-`0x100` | Clanker's moving parts, in `CC_INSIDE_CLANKER` only |

The bottom two rows only take effect in the map they name, so copying one elsewhere does nothing. Tumblar's shed shows the pairing at its simplest: `MMM_TUMBLARS_SHED` holds one model prop, at `(17, 0, -32)`, and one `0x38` node, at `(23, 128, -17)`, close enough for the game to join them.

## Collectibles and their save flags

A collectible doesn't know which one it is. It looks up the collectible ID zone it's standing in and takes its identity from that. Move one without its zone and it starts setting a different save flag, or stops counting at all. [Areas](AREAS.md) covers the zones.
