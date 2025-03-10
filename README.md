this whole project will be refactored/cleaned up, but functionality comes first

### Baserom checksums

- `baserom.us.v10.z64`: `1fe1632098865f639e22c11b9a81ee8f29c75d7a`
- `baserom.us.v11.z64`: `ded6ee166e740ad1bc810fd678a84b48e245ab80`
- `baserom.jp.z64`:     `90726d7e7cd5bf6cdfd38f45c9acbf4d45bd9fd8`
- `baserom.pal.z64`:    `bb359a75941df74bf7290212c89fbc6e2c5601fe`

# Building

The following instructions should work on the following platforms:
- Ubuntu 18.04 or higher (x86_64)

Building Instructions Table Of Contents:
- [Local (Linux)](#local-linux)

## Local (Linux)

Works with Ubuntu 18.04 or higher.

### 1. Install dependencies

```sh
sudo apt-get update && sudo apt-get install -y $(cat packages.txt)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
git submodule update --init --recursive
python3 -m venv myenv
source myenv/bin/activate
pip install -r requirements.txt
```

### 2. Add baserom

Add the file for `US v1.0` as `baserom.us.v10.z64` in the project folder.

(optional): Check the baserom checksum

```sh
sha1sum baserom.us.v10.z64
```

The output should match the checksum specified above.

### 3. Build

To extract and build everything simply run:

```sh
make
```

If you want to build a specific module, instead do:

```sh
make <module_id>
```

...where the following are supported values of `<module_id>`
- `core1`
- `core2`
- `MM`
- `TTC`
- `CC`
- `BGS`
- `FP`
- `lair`
- `GV`
- `CCW`
- `RBB`
- `MMM`
- `SM`
- `fight`
- `cutscenes`

### Version Selection

Drop in `us.v10` `us.v11`, `jp`, or `pal` as `baserom.<version>.z64` e.g. `baserom.us.v11.z64`

```sh
make VERSION=us.v11
```


## Port Building / testing
(on linux, might also work from WSL)

## the port itself
(run from the root folder)
```sh
cmake -S . -B cmake_build
cmake --build cmake_build 2>&1 | tee build_output.log
```

## building the rom to make sure code hasnt broken the rom

(run from the root folder)
```sh
source myenv/bin/activate
make clean; make 2> makefileoutput.txt
```
