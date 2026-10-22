# Reference

The format itself. The other pages cover what to do with it.

## The file

A setup file is a stream of tagged sections (`gsworld_load` walks it as the map loads):

| Tag | Section |
|---|---|
| `0x01` | Objects - the cube grid holding all props and nodes |
| `0x02` | Unused, skipped by the reader |
| `0x03` | Camera nodes |
| `0x04` | Lighting |
| `0x00` | End of file |

One more shape exists: a cube can list its nodes as 12-byte records instead of the 20-byte ones described below, and the reader accepts either.

The file opens with the grid bounds: min and max cell coordinates on each axis. Cells are 1000x1000x1000 world units; everything in section 1 is bucketed into this grid so the engine only tests what's near Banjo.

## Node fields

A node is an invisible point with a category, an id, and a few parameters. The YAML names each field, and leaves one out when it holds its default, usually 0:

| Field | Meaning |
|---|---|
| position (`Position`) | whole-number x/y/z |
| radius (`Radius`) | up to 511 units - a horizontal radius for the zone categories (the zone is a cylinder), a box half-extent for the contact categories; for an actor spawn it lands in the actor's `actorTypeSpecificField` as a free parameter |
| category (`Category`) | what kind of node this is |
| id (`Id`) | actor id, camera index, warp index, flag value... meaning set by the category |
| marker id (`Marker`) | which actor kind can set this node off |
| yaw (`Yaw`) | 0-359 degrees |
| scale (`Scale`) | for an actor spawn, x100 = 1.0; other systems repurpose it (a camera-controller node's scale is read back as a distance, and one dynamic camera mode switches on it outright - `dynamicCam12.c`) |
| chain links (`Number`, `Next`) | the node's own chain number (up to 4095) and that of the *next* node - the linked list the path system is built from. Every actor spawn resolves its chain and keeps the id as `secondaryId` |
| land/water setting (`ActiveWhen`) | camera zones only, four values - see [Areas](AREAS.md) |

## Categories

| Category | Name | In the YAML | Page |
|---|---|---|---|
| 3 | Warp (an exit) | `Warp` | [Warps](WARPS.md) |
| 4 | Contact trigger | `ContactTrigger` | [Areas](AREAS.md) |
| 6 | Actor | `Actor` | [Actors](ACTORS.md) |
| 7 | Enemy area | `EnemyArea` | [Areas](AREAS.md) |
| 8 | Path point | `PathPoint` | [Paths](PATHS.md) |
| 9 | Camera zone | `CameraZone` | [Areas](AREAS.md) |
| A | Collectible ID zone | `CollectibleZone` | [Areas](AREAS.md) |

Categories 0, 1, 2 and 5 are read and dispatched the same way, but every one of them lands on an empty case. They still take up a slot, which matters for the limit below.

A record whose category is none of these is **not a node at all**. The same slot also stores scripted path waypoints, which the game tells apart by a flag, not by the category. Read as a node, a waypoint shows a junk category and id, sometimes landing on a real category value by chance. Torch goes by the flag and writes them as `Category: Waypoint`.

## Limits

**Nodes are re-ordered inside a cube.** On load, each cube's nodes are partitioned. Categories 6 to A and every waypoint fill from the back of the array; categories 0 to 5 fill from the front. Only that front group is scanned when something touches a node. File order is not kept.

**A cube holds at most 31 nodes in that front group.** The count was a 5-bit field on the N64, so a 32nd wrapped it. The port widens the field and warns instead. Inert categories count toward it, so a cube can reach the limit without holding 31 working exits.
