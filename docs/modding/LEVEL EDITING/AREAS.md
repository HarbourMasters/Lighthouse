# Areas

Three node categories mark out **areas** rather than points: camera zones (9), enemy areas (7), and collectible ID zones (A). They have nothing to show in game, and all three work the same way. Contact triggers (4) mark out a region too, by a different mechanism, and this page covers them as well.

## How an area is built

Each node is a **cylinder**: its position plus a horizontal radius. Nodes that share an id and whose circles touch, seen from above, merge into one **zone**.

**An area of any shape is several overlapping nodes sharing an id.** To cover an L-shaped room, drop three or four nodes down its length rather than reaching for one enormous radius - the radius maxes out at 511 units anyway.

Spiral Mountain's water is the extreme case. Camera zone id 38 is not one big cylinder but dozens of them, most at radius 500, laid end to end along the water. Every one of them selects camera 38, so they are a single zone shaped like the water.

## Changing the camera in one spot (category 9)

The id is an index into the [camera list](CAMERAS.md). Banjo is in a zone when he's inside one of its cylinders and no more than 150 units above or below its node. That zone's camera takes over. Outside every zone he gets the ordinary free camera, and while flying the zones are ignored.

**Land or water.** A camera zone carries one more setting, deciding which of those two states it answers in. Banjo counts as swimming from the moment he enters water until he is back on stable ground, climbing or flying:

| Setting | On land | Swimming |
|---|---|---|
| 0 | yes, within 150 units | yes, at any height |
| 1 | yes, within 150 units | no |
| 2 | no | yes, at any height |
| 3 | no | yes, within 150 units |

Spiral Mountain uses three of the four. Most of its water cylinders are setting 2 - swimming only, any depth - so that camera holds however deep you dive. Zones 20 to 28 climb from height 362 to 923 and are setting 1 throughout, so none of them fire while swimming. The YAML calls it `ActiveWhen`.

Every camera starts enabled, and actors and cutscenes can switch one off and back on.

When the camera changes somewhere you didn't expect, find the category-9 node whose cylinder you stepped in, read its id, and look it up in the camera list.

## Starting and stopping an area camera (category 4)

A **contact trigger** is the other way a region changes the camera, and it works nothing like the cylinders above. It fires the same way a warp (category 3) does, whenever something touches it, and its radius reaches the same distance up, down and to every side, so it covers a box. A camera zone holds its camera only while you stand in it; a contact trigger flips a switch as you cross it and leaves it flipped.

Retail uses the ids in four groups:

| Ids | What |
|---|---|
| `0x16`-`0x29` | start the area camera - dynamic camera mode 12 - for area id `- 0x16` |
| `0x2A` | end the area camera |
| `0x46`-`0x4B` | treasure hunt step progress (TTC's x-marks-the-spot game) |
| `0x4C`, `0x4D` | Mumbo transformation boundary - warn, then detransform |

Every other id in the table does nothing.

Spiral Mountain uses only the first two rows: `0x16` nodes turn area camera 0 on, and `0x2A` nodes turn it off.

## Fencing an enemy in (category 7)

An enemy looks up which enemy area it's standing in and records it, and its movement code then refuses any position outside that zone. This is why an enemy gives up a chase at an invisible line. Mumbo's Mountain shows the shape of it. Its four Grublins each stand inside an enemy area built from overlapping 500-unit cylinders - the one at `(4025, 23, 3145)` sits inside two cylinders of zone id 2. That pair of circles is the whole of its patrol.

An enemy placed outside every enemy area isn't fenced at all. If you move an enemy, check it's still inside its zone.

## Telling a collectible which one it is (category A)

The id here is a value collectibles read to learn *their own identity*. A jiggy finds the collectible ID zone it stands in and its save flag is the zone's value plus 1; an empty honeycomb subtracts 99; a Mumbo token subtracts 199.

Mumbo's Mountain's highest jiggy, at `(373, 4221, -906)`, stands inside a collectible ID zone with id 1 and radius 127, so its save flag is 2.

The lookup ignores height, and where zones overlap, the one loaded first wins.
