# Warps

A **warp** is two halves in two different setups: an **exit** in the map you leave, and an **entrance** in the map you arrive at. Both have to exist or the warp misbehaves.

## The half you leave: exits

An exit is a category-3 node that fires when something **touches** it. Its radius reaches the same distance up, down and to every side, so it covers a box. Its *marker id* decides what can set it off. Most exits answer to Banjo, but Mumbo's Mountain's three orange pads (exit ids `0x2`, `0x3` and `0x4`) carry marker id 12, `MARKER_C_ORANGE_PROJECTILE`, so only a thrown orange sets them off and walking onto one does nothing. Contact triggers (category 4) are touched the same way but don't warp; see [Areas](AREAS.md).

**An exit's id decides where it goes.** Each id is one entry in a table of handlers, one per door, hut, lobby link and Nintendo-logo transition: `warp_mmEnterMumbosHut`, `warp_lairEnterCCLobbyFromCCLevel`, and so on. A handler carries its destination map and entrance, so the same exit id always leads to the same place. Romhacks re-point them through the game config instead: each `Warps` row is an exit id with the `Map` and `Entrance` it leads to.

**Leave an exit's yaw and scale alone.** The game keeps a timer in those two fields to stop the exit firing every frame, and overwrites them the first time anything touches it. Banjo's facing comes from the entry point at the other end.

## The half you arrive at: entrances

Every transition carries an **entrance number**. On arrival the game turns that number into an entry-point actor id and looks for a node carrying it: entrance 1 wants actor `0x01`, entrances 4-13 want `0x76`-`0x7F`, entrance `0x12` wants `0x103`, and so on. Banjo is placed at that node, facing its yaw.

**An entrance is a pair of nodes, not one.** One puts Banjo somewhere; the other tells the camera where to be, taking a distance from its scale field. A second table turns the same entrance number into that entry *camera* actor id: `0x80` for entrance 0, counting up from there, with other ids further down the table. The load also spawns the camera-controller actor `0x66`.

### When it goes wrong

If the entry point the entrance asks for is missing, the game walks entrance numbers `0` to `0x1D` in order and takes the first one the map does have. So:

| The destination map has | What happens |
|---|---|
| the entry point the exit asks for | Banjo arrives where you meant |
| some other entry point in the `0`-`0x1D` range | Banjo arrives at *that* one instead - the warp works, but comes out in the wrong place |
| no entry point in that range | Banjo is left at the void default and falls forever (the port logs it at debug level) |

Entrance numbers of `0x80` and above are a different mechanism - they carry their own position and need no node. `0x63` and `0x65` are answered before the lookup happens too.

## A whole warp: Banjo's front door

Both halves, in both directions, are four records.

**Going in.** Spiral Mountain carries an exit with id `0x118` at `(3926, -425, 6761)`, radius 157. Touching it runs `warp_smEnterBanjosHouse`, which asks for `0x8C01` - map `0x8C` and entrance `1`. Banjo's house is map `0x8C`, and it carries an actor node with id `0x1` at `(0, 0, 157)` facing yaw 180. Entrance 1 asks for actor `0x1`, so that node is where Banjo lands, facing into the room.

**Coming back.** The house carries one exit of its own, id `0x119` at `(-7, 74, 452)`, running `warp_smExitBanjosHouse` for `0x112` - map `0x1`, entrance `0x12`. Entrance `0x12` asks for actor `0x103`, and Spiral Mountain has one, at `(3955, -490, 6484)` facing yaw 180.

## Adding a warp to a new area

1. Place an **exit node** where the player should touch it, and give it the id of a handler that leads where you want - or re-point an existing one under the game config's `Warps`.
2. In the destination map, place an **entry point** actor node for the entrance number that exit uses, facing where Banjo should be looking.
3. Place the matching **entry camera** node beside it.
4. Add a return exit the same way, and an entry point back in the first map.
