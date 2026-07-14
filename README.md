[comment]: <> (Todo: Make Light Mode Image)
[comment]: <> (Todo: Make Dark Mode Image)

# Lighthouse

Lead Developers: 
* Malkierian

## Discord

Official Discord: https://discord.com/invite/shipofharkinian

If you're having any trouble after reading through this `README`, feel free ask for help in the Lighthouse Support text channels. Please keep in mind that we do not condone piracy.

# Quick Start

Lighthouse does not include any copyrighted assets.  You are required to provide a supported copy of the game.

### 1. Verify your ROM dump
The supported ROMs are US 1.0 and US 1.1 Rev A versions. You can verify you have dumped a supported copy of the game by using the SHA-1 File Checksum Online at https://www.romhacking.net/hash/. 

* `baserom.us.v10.z64`: `1fe1632098865f639e22c11b9a81ee8f29c75d7a`
* `baserom.us.v11.z64`: `ded6ee166e740ad1bc810fd678a84b48e245ab80`
* `baserom.jp.z64`:     `90726d7e7cd5bf6cdfd38f45c9acbf4d45bd9fd8`
* `baserom.pal.z64`:    `bb359a75941df74bf7290212c89fbc6e2c5601fe`

If you have multiple regions of the game and want to use them as language packs, see `# Language Packs` below.

### 2. Verify your ROM is in .z64 format
Your ROM needs to be in .z64 format. If it's in .n64 format, use the following to convert it to a .z64: https://hack64.net/tools/swapper.php

### 2. Download Lighthouse from [Releases](https://github.com/HarbourMasters/Lighthouse/releases)

### 3. Generating the OTR from the ROM and Play!
#### Windows
* Extract every file from the zip into a folder of your choosing.
* Run lighthouse.exe and select your compatible ROM.

#### Linux
* Extract every file from the zip into a folder of your choosing.
* Execute lighthouse.appimage. You may have to chmod +x the appimage via terminal.

#### MacOS
* Extract every file from the zip into a folder of your choosing.
* Run lighthouse and select your compatible ROM.

#### Nintendo Switch
* Run one of the PC releases to generate an `lighthouse.o2r` file. After launching the game on PC, you will be able to find these files in the same directory as `lighthouse.exe` or `lighthouse.appimage`.
* Copy the files to your sd card

# Configuration

### Default keyboard configuration
| N64 | A | B | Z | Start | Analog stick | C buttons | D-Pad |
| - | - | - | - | - | - | - | - |
| Keyboard | X | C | Z | Space | WASD | Arrow keys | TFGH |

### Other shortcuts
| Keys | Action |
| - | - |
| ESC | Toggle menubar |
| F4 | Reset |
| F11 | Fullscreen |
| Tab | Toggle Alternate assets |

### Graphics Backends

TODO: verify this info

Currently, there are three rendering APIs supported: DirectX11 (Windows), OpenGL (all platforms), and Metal (macOS). You can change which API to use in the `Settings` menu of the menubar, which requires a restart.  If you're having an issue with crashing, you can change the API in the `lighthouse.cfg.json` file by finding the line `"Backend":{`... and changing the `id` value to `3` and set the `Name` to `OpenGL`. `DirectX 11` with id `2` is the default on Windows. `Metal` with id `4` is the default on macOS.

# Language Packs

Lighthouse supports using multiple regions of Banjo-Kazooie as language packs. Generate your base `bk.o2r` file and once Lighthouse has started, open the imgui menu. Under General -> Languages, you will be able to select another ROM to extract as a language pack. PAL supports UK, French, and German, and Japanese adds Japanese support. After the language pack is generated, the language it brings will be added to the dropdown menu.

When using romhacks, only US v1.0 is supported, inherited from Banjo's Backpack. Therefore, it is recommended to use **US v1.0** as your base `bk.o2r` file.

# Custom Assets
Custom assets are packed in `.o2r` or `.otr` files. To use custom assets, place them in the `mods` folder.

If you're interested in creating and/or packing your own custom asset `.o2r`/`.otr` files, check out the following tools:
* [**retro - OTR and O2R generator**](https://github.com/HarbourMasters64/retro)
* [**fast64 - Blender plugin (Note that SF64 is not supported at this time)**](https://github.com/HarbourMasters/fast64)

# Development
### Building

TODO: add a main branch

If you want to manually compile Lighthouse, please consult the [building instructions](https://github.com/HarbourMasters/Lighthouse/blob/main/docs/BUILDING.md).

### Playtesting
If you want to playtest a continuous integration build, you can find them at the links below. Keep in mind that these are for playtesting only, and you will likely encounter bugs and possibly crashes. 

TODO: replace these with lighthouse builds

* [Windows](https://nightly.link/HarbourMasters/Starship/workflows/main/main/starship-windows.zip)
* [macOS](https://nightly.link/HarbourMasters/Starship/workflows/main/main/starship-mac-x64.zip)
* [Linux](https://nightly.link/HarbourMasters/Starship/workflows/main/main/Starship-linux.zip)
* [Switch](https://nightly.link/HarbourMasters/Starship/workflows/main/main/Starship-switch.zip)

<a href="https://github.com/Kenix3/libultraship/">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="./docs/poweredbylus.darkmode.png">
    <img alt="Powered by libultraship" src="./docs/poweredbylus.lightmode.png">
  </picture>
</a>

# Special Thanks:

* GiantJigglypuff3 for their extensive and continued Banjo Decomp documentation  
TODO: fill in
