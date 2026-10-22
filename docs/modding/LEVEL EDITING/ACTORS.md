# Actors

Anything that moves, talks, hurts you or can be collected is an **actor**, and an actor is placed by a node, not a prop.

## Placing one

A category-6 node spawns its id as an actor, at the node's position and yaw: a Grublin node puts a Grublin there. Two of the node's other fields are handed to the actor as free parameters, and what they mean is up to that actor:

- **radius**, which turns up in the actor's `actorTypeSpecificField`
- **scale**, where 100 means normal size, the same as on a prop; leave it at 0 and the actor uses its default size

The node's own chain number also reaches the actor, which is how a path finds its rider. See [Paths](PATHS.md).

An actor only spawns in a world that registers its kind. Each world registers the kinds it uses as it loads, before it spawns the map's nodes, so an actor placed in a world that doesn't register it never appears. With a romhack loaded, Lighthouse registers every kind on every map.

**Every actor in the map spawns at load**, all cubes at once, though anything outside the player's range is culled and may not update until Banjo gets closer.

## What the free parameter is for: Boggy's slalom

Freezeezy Peak builds Boggy's course out of 78 actor nodes that are identical but for one number. Two actor ids, `0x161` and `0x162`, are the left and right posts of a gate; each is placed 39 times, and the only field that differs between one flag and the next is the radius - **1 on the first pair, counting up to 39 on the last**.

That number is the gate's place in the race. A flag hands it to the race code when somebody passes between the posts. The race records the gate as the player's or Boggy's and spawns the gates four places further along.

The race also counts its flags before it will run: it waits for all 78 to register, and holds if any are missing.

## Actors that find each other: Mr. Vile's game

Some actors are placed knowing that another actor will come looking for them. Bubblegloop Swamp's Mr. Vile arena (`BGS_MR_VILE`, map `0x10`, `ASSET_72C_UNNAMED.yaml`) is three actor ids:

| Node | Where | How many |
|---|---|---|
| `0x139` `YUMBLIE` | a grid across the floor, roughly 350 units apart, all at height -100 | 25 |
| `0x138` `VILE_GAME_CTRL` | `(-397, 125, -1257)`, off to one side | 1 |
| `0x13A` `MR_VILE` | `(0, 0, 0)`, facing yaw 180 | 1 |

Every one of the 25 holes is the same record: radius 50, yaw 0, no chain, no parameter. On its first update each one searches the live actor list for the **nearest** `VILE_GAME_CTRL` and keeps that as its controller; when it pops up it hands the controller its own position. The controller never reads the setup - it learns the board from whoever reports in. So adding a hole is placing one more `0x139` anywhere on the floor.

The rest is in code: how many holes are up at once, whether each comes up as a Yumblie or a Grumblie, and how long it stays. Mr. Vile himself goes for the nearest piece of the wanted kind, and when there is none he wanders a box 500 units either side of his own node - a smaller box than the grid, which reaches 800. The camera for the game is computed rather than taken from a camera node.

## A number that indexes a table: the jigsaw podium

Every jiggy podium in Gruntilda's Lair is one actor node - `0x3B7`, or `0x3BC` for the second style of podium - and its radius is the **puzzle number**, 1 to 11.

The number is an index into a table (`jigsawpicture.c`) holding three things per puzzle: the cost in jiggies, how much room the save file gives the count, and which save flag it is. The costs run 1, 2, 5, 7, 8, 9, 10, 12, 15, 25 and 4. A romhack's game config can override every row, under `JiggyPuzzles`.

The number does two more jobs. For puzzles 1 to 9, finishing the picture sets level flag `number + 0x1B`, which is the flag that world's door watches; 10 opens Grunty's door and 11 grants double health instead. The order the pieces fly in is seeded from it, so each picture assembles the same way every time.

A number outside 1 to 11 has no row, and the podium treats it as costing nothing. A twelfth world needs a twelfth row in the table before it needs a podium.

**Changing one.** Say Mumbo's Mountain should cost 4 jiggies instead of 1. The podium node stays exactly as it is, radius 1 still meaning row 1. The change goes in three other places:

1. **The cost.** Set row 1's `Cost` to 4 in the game config's `JiggyPuzzles`. That list counts from 0, so row 1 is keyed `0`.
2. **Room in the save file.** The same row's `Size` gives the count one bit, and a count of 4 needs three. Torch doesn't catch this. The counts sit back to back from `0x5D` to `0x82` with no free space past the end, so three bits at `0x5D` would overwrite Treasure Trove Cove's. Take the two extra bits from other pictures instead: dropping Treasure Trove Cove's cost to 1 and double health's to 3 frees one bit each. Then move each row's `Flag` so the rows sit end to end again.
3. **The pieces.** Each piece is a vertex group in the lobby's map model (not the podium), numbered 400 plus the piece's index in this puzzle. An unplaced piece draws invisible and a placed one solid. The Mumbo's Mountain lobby has one piece, group 400; a cost of 4 wants 400 through 403, each carrying its own slice of the image. In Blender those are vertex groups named `bk64_mesh_400` and so on, on the lobby's map model - see the plugin's [Mesh Lists](https://github.com/HarbourMasters/fast64/blob/main/fast64_internal/hm64/bk64/README.md#mesh-lists) section. A new picture is the same groups with a different texture.

Skip step 3 and the puzzle still works: the door opens at four jiggies, but three of them never appear on the wall.

## The invisible actors

Some actors have no model at all. They give a *place* a behavior for another actor to read.

**Wozza's retreat.** Freezeezy Peak places Wozza (`0x1F3`) at `(-5130, 800, -4499)`, and he never stands there. As he spawns he looks up four other actor ids in the same setup by number, keeps their positions, and moves himself onto one of them:

| Node | Position | Action |
|---|---|---|
| `0x35B` | `(-5037, 800, -4348)` | stands here |
| `0x359` | `(-5546, 792, -4170)` | first stop when he runs |
| `0x35A` | `(-5855, 793, -4058)` | second stop, where he waits |
| `0x35C` | `(-6144, 795, -3948)` | what he turns to face while waiting |

Those four are the whole route. He runs west toward his cave (`warp_fpEnterWozzasCave`, id `0xFD`) and stops short of the exit. None of the four has a model.

**Climb markers.** A climbable tree or pole is a pair of nodes: `0x26` (`ACTOR_26_CLIMB_BASE`) at the bottom, and a top marker - `0x27` or `0x28` - above it. The base actor finds the nearest top of either kind and the two ends define the climbable span; the base node's radius is the climb radius. The two top kinds differ in one thing: reach the top of a `0x28` climb moving upward and Banjo climbs out onto whatever is above. A `0x27` top just ends the pole.

**Entry points.** Where Banjo lands when he arrives from another map. These come in pairs with a camera node - see [Warps](WARPS.md).

**Bobbers and movers.** A handful of ids hand a nearby piece of scenery over to code that moves it - floating platforms, Tumblar, Clanker's parts. See [Scenery that moves](OBJECTS.md#scenery-that-moves).

**Walk-in nodes** (`0x184`-`0x186`). When Banjo exits within 500 units of one (1000 for `0x186`), the transition plays the walk-out with the node's yaw and scale as parameters, or walks him to the node outright for `0x184`. Retail uses `0x185`: Spiral Mountain puts one at `(0, 1784, -3659)` facing yaw 180, beside the warp that runs `warp_lairEnterLairFromSMLevel`, so the two sit together as an exit and its walk-out. Retail ships no `0x186` at all.

**Map trigger nodes.** Ids `0x018`, `0x18A`, `0x18B` and `0x192`-`0x194` run code from a per-map table. `0x18B` compares only the player's height against the node's - no radius, no horizontal distance - and lifts the player back up to that height when they fall below it. Rusty Bucket Bay's engine room puts one at `(-61, -4792, 2570)`, so only the `-4792` matters: it is a floor under the whole room, not a spot in it.
