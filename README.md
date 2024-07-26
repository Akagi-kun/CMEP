# CMEP
[![Windows](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-windows.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-windows.yml) [![Linux](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-linux.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-linux.yml) [![MacOS](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-macosx.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-macosx.yml) <a href="https://scan.coverity.com/projects/snezhnaya-chan-cmep"><img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/29326/badge.svg"/></a>

This is the repository of the CMEP project. A simple scriptable game engine written in C++, using the Vulkan API for rendering.
This project is currently in alpha and although simple games (as shown in the `./examples/` subdirectory) can be made with it, is very simple and lacks in functionality.

---
### Usage

#### Install Vulkan SDK
Before you build the engine you first have to install the Vulkan SDK, download it from https://vulkan.lunarg.com/ and install it/unpack it.

Depending on your platform (Linux needs this, untested on MacOS), it might be necessary to set the `VULKAN_SDK` environment variable to a valid Vulkan SDK location so cmake finds all includes. Use either the `setup-env.sh` script (`source path/to/vulkansdk/setup-env.sh`) that comes with the Vulkan SDK or set it manually.

#### Clone repository and submodules
First use `git clone "https://github.com/Snezhnaya-chan/CMEP.git"` in a new clean directory to clone repository.

Then `git submodule init` and `git submodule update --recursive` to initialize and clone dependency submodules.

#### Project build
To build the core libraries and rungame executable use the `build.sh` and `build.bat` scripts depending on your platform.
Both can be used as follows:
```
./build.sh <CONFIG>
```
or
```
./build.bat <CONFIG>
```
> Where `<CONFIG>` can be omitted (which builds Debug by default) and if provided is a valid configuration value (Either `Debug` or `Release`, case-sensitive)

Optionally you can also build manually by directly invoking cmake (not recommended):
```
cmake .
cmake --build . --target rungame --config <CONFIG>
```
> Note: that although the configuration can be omitted here, it is not recommended as the default depends on build system used

#### Build an example

To build any of the examples, use cmake:
```
cmake .
cmake --build . --target <examplename>
```
> Where `<examplename>` is a name of a subdirectory under `./examples/` (e.g. `floppybirb`)
> Note: examples currently do not use the `--config` syntax and therefore it isn't necessary to provide it

#### Running
To start the engine use the `rungame` executable. This is located under the `./build/` subdirectory if the build is successful. A `./build/game/` directory with valid content is necessary for startup, this can be created by building an example.

---

### Dependencies
This project depends on:
- Lua programming language
- GLFW library
- lodepng
- tinyobjloader
- nlohmann-json
- Vulkan SDK

All except the Vulkan SDK are prepackaged or built automatically once the build script is run.
These projects are external to this one and are not affiliated with this project in any way, when using this project take care to follow licenses of the dependencies.

---

### License
This project is licensed under the MIT license (see the `LICENSE` file for more information). For licenses of the dependencies, see the `./external/` directory 
