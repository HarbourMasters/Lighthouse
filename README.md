[comment]: <> (Todo: Make Light Mode Image)
[comment]: <> (Todo: Make Dark Mode Image)

# Lighthouse
Harbour Masters port of Banjo Kazooie

Lead Developer: 
* Malkierian

Developers:
* JeodC
* Caladius

## Discord
Official Discord: https://discord.com/invite/shipofharkinian

If you're having any trouble after reading through this `README`, feel free ask for help in the Lighthouse text channels. Please keep in mind that we do not condone piracy.

# Quick Start

Lighthouse does not include any copyrighted assets.  You are required to provide a supported copy of the game.

### 1. Verify your ROM dump
Any retail version listed below is supported. You can verify you have dumped a supported copy of the game by using the SHA-1 File Checksum Online at https://www.romhacking.net/hash/. 

* `baserom.us.v10.z64`: `1fe1632098865f639e22c11b9a81ee8f29c75d7a`
* `baserom.us.v11.z64`: `ded6ee166e740ad1bc810fd678a84b48e245ab80`
* `baserom.jp.z64`:     `90726d7e7cd5bf6cdfd38f45c9acbf4d45bd9fd8`
* `baserom.pal.z64`:    `bb359a75941df74bf7290212c89fbc6e2c5601fe`

If you have multiple regions of the game and want to use them as language packs, see `# Language Packs` below.

### 2. Verify your ROM is in .z64 format
Your ROM needs to be in .z64 format. If it's in .n64 format, use the following to convert it to a .z64: https://hack64.net/tools/swapper.php

### 3. Download Lighthouse from [Releases](https://github.com/HarbourMasters/Lighthouse/releases)

### 4. Generating the OTR from the ROM and Play!

#### Windows
* Extract every file from the zip into a folder of your choosing.
* Run lighthouse.exe and select your compatible ROM.

#### Linux
* Extract every file from the zip into a folder of your choosing.
* Execute lighthouse.appimage. You may have to chmod +x the appimage via terminal.

#### MacOS
* Extract every file from the zip into a folder of your choosing.
* Run lighthouse and select your compatible ROM.

# Configuration

Lighthouse ships with a file with many standard controller mappings that can be used as-is with most controllers. If your controller isn't recognized by Lighthouse, or isn't working properly, you can create your own custom mapping using the built-in mapper in the Settings menu.

### Default keyboard configuration
| N64 | A | B | L | R | Z | Start | Analog stick | C buttons | D-Pad |
| - | - | - | - | - | - | - | - | - | - |
| Keyboard | X | C | E | R | Z | Space | WASD | Arrow keys | TFGH |

### Other shortcuts
| Keys | Action |
| - | - |
| ESC | Toggle menubar |
| Ctrl+R / ⌘R | Reset |
| F11 | Fullscreen |
| Tab | Toggle Alternate assets |

### Graphics Backends
Currently, there are three rendering APIs supported: DirectX11 (Windows), OpenGL (all platforms), and Metal (macOS). You can change which API to use in the `Settings` menu of the menubar, which requires a restart.  If you're having an issue with crashing, you can change the API in the `lighthouse.cfg.json` file by finding the line `"Backend":{`... and changing the `id` value to `2` and set the `Name` to `OpenGL`. `DirectX 11` with id `1` is the default on Windows. `Metal` with id `3` is the default on macOS.

# Language Packs

Lighthouse supports using multiple regions of Banjo-Kazooie as language packs. Generate your base `bk.o2r` file and once Lighthouse has started, open the imgui menu. Under General -> Languages, you will be able to select another ROM to extract as a language pack. PAL supports UK, French, and German, and Japanese adds Japanese support. After the language pack is generated, the language it brings will be added to the dropdown menu.

# Romhacks

Many romhacks can be extracted from patched ROMs and used as mods with Lighthouse. This can be done in-game using the Romhacks menu in the Settings section. If any hack isn't fully supported, the menu should inform you when extracting the mod. Most supported romhacks currently haven't been fully tested, so there may still be some issues.

_NOTE: When using romhacks, only US v1.0 is supported, inherited from Banjo's Backpack. Therefore, it is recommended to use **US v1.0** as your base `bk.o2r` file.

# Custom Assets

Custom assets are packed in `.o2r` or `.otr` files. To use custom assets, place them in the `mods` folder.

If you're interested in creating and/or packing your own custom asset `.o2r`/`.otr` files, check out the following tools:
* [**retro - OTR and O2R generator**](https://github.com/HarbourMasters64/retro)
* [**fast64 - Blender plugin (Note that BK64 is not supported at this time)**](https://github.com/HarbourMasters/fast64)

# Development

### Building
If you want to manually compile Lighthouse, please consult the [building instructions](https://github.com/HarbourMasters/Lighthouse/blob/main/docs/BUILDING.md).

### Playtesting
If you want to playtest a continuous integration build, you can find them at the links below. Keep in mind that these are for playtesting only, and you will likely encounter bugs and possibly crashes.

* [Windows](https://nightly.link/HarbourMasters/Lighthouse/workflows/main/main/Lighthouse-windows.zip)
* [macOS](https://nightly.link/HarbourMasters/Lighthouse/workflows/main/main/Lighthouse-mac.zip)
* [Linux](https://nightly.link/HarbourMasters/Lighthouse/workflows/main/main/Lighthouse-linux.zip)

<a href="https://github.com/Kenix3/libultraship/">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="./docs/poweredbylus.darkmode.png">
    <img alt="Powered by libultraship" src="./docs/poweredbylus.lightmode.png">
  </picture>
</a>

# Special Thanks:

* The Banjo decomp team
* Fredomato, scorched11 for work on rando
