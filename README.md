# CMEP
[![Windows](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-windows.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-windows.yml) [![Linux](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-linux.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-linux.yml) [![MacOS](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-macosx.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-macosx.yml) <a href="https://scan.coverity.com/projects/snezhnaya-chan-cmep"><img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/29326/badge.svg"/></a>

This is the repository of the CMEP project. A scriptable game engine written in C++, using the Vulkan API for rendering.
This project is currently in early stages of development and although basic demos (as shown in the `./examples/` subdirectory) can be made with it, the API often goes through changes and lacks in functionality.

---
## Usage

### Install Vulkan SDK
Before you build the engine you first have to install the Vulkan SDK, download it from https://vulkan.lunarg.com/ and install it/unpack it.

Depending on your platform (Linux needs this, untested on MacOS), it might be necessary to set the `VULKAN_SDK` environment variable to a valid Vulkan SDK location so cmake finds all includes. Use either the `setup-env.sh` script (`source path/to/vulkansdk/setup-env.sh`) that comes with the Vulkan SDK or set it manually.

### Clone repository and submodules
First use `git clone "https://github.com/Snezhnaya-chan/CMEP.git"` in a new clean directory to clone repository.

Then `git submodule init` and `git submodule update --recursive` to initialize and clone dependency submodules.

### Project build
To build the core libraries and rungame executable use the `build.sh` and `build.bat` scripts depending on your platform.
Both can be used as follows:
```
./build.sh <CONFIG>
```
or
```
./build.bat <CONFIG>
```
> [!IMPORTANT]
> Where `<CONFIG>` can be omitted (which builds Debug by default) and if provided is a valid configuration value (Either `Debug` or `Release`, case-sensitive)

You can also build manually by directly invoking cmake (useful if the build scripts do not work for you):
```
cmake -DCMAKE_BUILD_TYPE=<CONFIG> .
cmake --build . --target rungame --config <CONFIG>
```
> [!TIP]
> It is not necessary to specify *both* CMAKE_BUILD_TYPE (only used by Makefile generators) and --config (only used by multi-config generators)
> you may instead specify only the one your generator uses

> [!WARNING]
> By default most generators build the Debug configuration if none is specified, this may not be what you want 

### Build an example

To build any of the examples, use cmake:
```
cmake --build . --target <examplename>
```
> [!IMPORTANT]
> Where `<examplename>` is a name of a subdirectory under `./examples/` (e.g. `floppybirb`)

> [!NOTE]
> Examples do not use the `--config` syntax and therefore it isn't necessary to provide it

### Running
To start the engine use the `rungame` executable. This is located under the `./build/` subdirectory if the build is successful. A `./build/game/` directory with valid content is necessary for startup, this can be created by building an example.

### Building documentation
HTML documentation can be generated using doxygen, the `build_docs` cmake target is provided for this purpose, output is by default generated in the `./docs/output` directory.

---
## Dependencies
This project depends on:
- LuaJIT and the Lua programming language
- GLFW library
- lodepng
- tinyobjloader
- nlohmann-json
- Vulkan SDK
- glslang, SPIRV-Tools and SPIRV-Headers

All except the Vulkan SDK are prepackaged or built automatically once the build script is run.
These projects are external to this one and are not affiliated with this project in any way, when using this project take care to follow licenses of the dependencies.

---
## License
This project is licensed under the MIT license (see the `LICENSE` file for more information). For licenses of the dependencies, see the `./external/` directory 
