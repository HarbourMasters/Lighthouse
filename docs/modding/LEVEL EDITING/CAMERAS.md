# Cameras

A setup carries a numbered list of cameras, up to 70 of them. A [camera zone](AREAS.md) hands one of them control by index, and the camera's **type** decides what it then does.

| Type | Name | What it does | Example |
|---|---|---|---|
| 1 | Pivot | mounted at a spot, turns to follow Banjo | `MM_MUMBOS_MOUNTAIN` camera 9 |
| 2 | Static | position and angle both fixed; nothing follows anything | `SM_SPIRAL_MOUNTAIN` camera 1 |
| 3 | Dolly | slides along the line toward Banjo, staying between two distances | `SM_SPIRAL_MOUNTAIN` camera 0 |
| 4 | Zoom preset | frames nothing; re-tunes how far back the ordinary camera sits | `SM_SPIRAL_MOUNTAIN` camera 38 |

In the YAML, `Type` takes the name, with type 4 written `ZoomPreset`.

A zone pointing at an index with no camera falls back to the free camera, which is what you get if you delete a camera and leave its zones behind.

## Where a camera sits and what it looks at

Every camera except type 4 carries a **position** and three more numbers the file calls pitch, yaw and roll. Only a static camera reads them as angles. Torch names them by that use: `PitchYawRoll` on a static camera, `LooksFrom` on a pivot or dolly.

- On a **static** camera that's all there is. The camera sits at the position, points at the angles, and never moves. Nothing tracks Banjo. Camera zones don't select these - scripted code cuts to one by index. Freezeezy Peak's camera 46 is one: a static at `(-3682, 1765, -5278)`, pitch 332 and yaw 131, that Wozza's code cuts to by number for both of his meetings, as a bear and as a walrus.
- On a **pivot** camera the position is the mount, and the three numbers are an offset from it to a point. The camera turns to keep Banjo in frame, and once he is within 100 units of it sideways, too close to give it a heading, it takes its heading from that point instead (`dynamicCam8.c`).
- On a **dolly** camera the position anchors the line it slides along, and the numbers mark a point the same way, used once Banjo is within 150 units of the anchor (`dynamicCam11.c`). The camera never sits farther from Banjo than the anchor is, so put the anchor at least the far distance from where he'll stand, or it can't pull back that far.

## Tuning how it moves

Pivot and dolly cameras carry four tuning numbers, in two pairs, plus the dolly's distances. Each camera has its own set, applied when its zone takes over:

| Setting | What it does |
|---|---|
| **horizontal / vertical speed** (`HorizontalSpeed`, `VerticalSpeed`) | how quickly the camera slides to keep up, sideways and up-down as separate rates |
| **rotation / acceleration** (`Rotation`, `Acceleration`) | how fast it turns toward where it wants to point, and how sharply it gets up to that rate |
| **close / far distance** (`CloseDistance`, `FarDistance`; dolly only) | the band it holds between. It slides in when Banjo is nearer than the close distance and out when he is past the far one |

Small values make the camera drift; large ones make it snap. Retail mostly does not tune them: most pivot cameras share horizontal 1.75, vertical 3.75, rotation 2.75 and acceleration 12, and a second common set is 0.70, 2.33, 4.00 and 16.00. If you have no feel for these numbers yet, start from the first set.

## Flags

Pivot cameras carry three flags; dolly cameras carry the first and the last:

- **hits** runs line-of-sight correction, so the camera is pushed out of geometry rather than ending up inside a wall. Leave it on unless the camera has a clear view by construction.
- **vfix** (pivot only) holds the authored height instead of tracking Banjo's. Use it when he can climb inside the shot and you want the camera to stay put. Every retail pivot camera sets it.
- **bee** marks the camera as still applying during the bee transformation. A zone whose camera lacks it is ignored while flying as the bee.

In the YAML they're the low bits of `Flags`: hits `0x1`, vfix `0x2`, bee `0x4`. Retail files carry leftover bits above them, which the game ignores.

## Zoom presets

Type 4 holds one number, `Preset`, and no position. When its zone takes over you stay on the ordinary camera, with its distances changed underneath you. Each world carries a table of zoom presets, keyed by map and by that number. A row holds three settings, one per step of the zoom button: how far back the camera sits, and how high it rides, at closest, medium and furthest.

Out of the water, Spiral Mountain's closest zoom sits 550 units back and 175 up. Its water cylinders all select camera 38, camera 38 holds the number 1, and the map's row 1 reads 800 and 375. Swim, and the camera drifts further out and higher without you touching the zoom button; climb out and it closes back in. Gobi's Valley and Rusty Bucket Bay use more rows.
