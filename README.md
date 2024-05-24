# CMEP
[![Windows](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-windows.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-windows.yml) [![Linux](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-linux.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-linux.yml) [![MacOS](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-macosx.yml/badge.svg)](https://github.com/Snezhnaya-chan/CMEP/actions/workflows/build-macosx.yml) <a href="https://scan.coverity.com/projects/snezhnaya-chan-cmep"><img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/29326/badge.svg"/></a> [![Coverage Status](https://coveralls.io/repos/github/Snezhnaya-chan/CMEP/badge.svg?branch=master)](https://coveralls.io/github/Snezhnaya-chan/CMEP?branch=master)

This is the repository of the CMEP project. A simple scriptable game engine written in C++, using the Vulkan API for rendering.
This project is currently in alpha and although games (as shown in the `./examples/` subfolder) can be made with it, is very simple and lacks in functionality.

### Install Vulkan SDK
Before you build the engine you first have to install the Vulkan SDK, download it from https://vulkan.lunarg.com/ and install it/unpack it.

Depending on your platform (Linux needs this, untested on MacOS), it might be necessary to set the `VULKAN_SDK` environment variable to point to Vulkan SDK so cmake finds all includes. Use either the `setup-env.sh` script that comes with Vulkan SDK or set it manually.

### Building
To build the core libraries and rungame executable use the `build.sh` and `build.bat` scripts depending on your platform or build manually using CMake:
```
cmake .
cmake --build . --target rungame
```


To build any of the examples, use cmake:
```
cmake .
cmake --build . --target <examplename>
```
where `<examplename>` is a name of a subdirectory under `./examples/` (e.g. `floppybirb`)

### Running
To start the engine use the `rungame` or `rungame.exe` executables depending on your platform. These are located under the `./build/` subdirectory after you build the project and an example.

### Dependencies
This project depends on:
- Lua programming language
- GLFW library
- lodepng
- nlohmann-json
- Vulkan SDK

all except the Vulkan SDK are prepackaged or built automatically once the build script is run.
These projects are external to this one and are not affiliated with this project in any way, when using this project take care to follow licenses of the dependencies.

### License
This project is licensed under the MIT license (see the `LICENSE` file for more information).
