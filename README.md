# Vortex Engine

[![Build](https://github.com/Vitriol1744/Vortex/actions/workflows/build.yml/badge.svg)](https://github.com/Vitriol1744/Vortex/actions/workflows/build.yml)


## Description
Low level game engine being written in modern C++23 utilizing powerful Vulkan API

## Dependencies
To be able to use Vortex you need to install a few dependencies:

* Modern C++ compiler, e.g: clang++-18
* Vulkan SDK 
* Platform specific window server dev tools

### Setup on Arch derivates
```
sudo pacman -S git cmake base-devel vulkan-devel clang ninja libx11 wayland libxinerama 
```

### Setup on Debian based distros
```
sudo apt update
sudo apt install -y build-essential ninja-build clang vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools libx11-dev libwayland-bin libxrandr-dev libxinerama-dev libxcursor-dev
wget https://apt.llvm.org/llvm.sh
sudo chmod u+x llvm.sh
./llvm.sh 18
```


## Building
The first thing you have to do is to sync shaderc dependencies and generate the CMake Cache for the build system of your choice, e.g. Ninja like this:
```
(cd Vortex-Engine/vendor/shaderc && ./utils/git-sync-deps) && cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++-18 -DCMAKE_BUILD_TYPE=Debug
```

Afterwards you can build the engine using:
```
cmake --build build
```

The binaries are stored inside the ./bin directory

## Support
Vortex currently supports Linux with wayland or Xlib and Windows

## Roadmap
You can find the roadmap at: https://trello.com/b/NwfDrxwK/vortex-engine


## Authors and acknowledgment
* v1tr10l7

### Credits
* [assimp](https://github.com/assimp/assimp.git) - Open Asset Import Library is a library to load various 3d file formats into a shared, in-memory format. It supports more than 40 file formats for import and a growing selection of file formats for export.
* [entt](https://github.com/skypjack/entt.git) - EnTT is a header-only, tiny and easy to use library for game programming and much more written in modern C++.
* [fmt](https://github.com/fmtlib/fmt.git) - {fmt} is an open-source formatting library providing a fast and safe alternative to C stdio and C++ iostreams.
* [glm](https://github.com/g-truc/glm.git) - OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications.
* [glfw](https://github.com/glfw/glfw.git) - GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development.
    It provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.
* [imgui](https://github.com/ocornut/imgui.git) - Dear ImGui is a bloat-free graphical user interface library for C++.
* [magic_enum](https://github.com/Neargye/magic_enum.git) - Header-only C++17 library provides static reflection for enums, work with any enum type without any macro or boilerplate code.
* [shaderc](https://github.com/google/shaderc.git) - A collection of tools, libraries and tests for shader compilation.
* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross.git) - SPIRV-Cross is a tool designed for parsing and converting SPIR-V to other shader languages.
* [stb](https://github.com/nothings/stb.git) - single-file public domain (or MIT licensed) libraries for C/C++
* [tracy](https://github.com/wolfpld/tracy.git) - A real time, nanosecond resolution, remote telemetry, hybrid frame and sampling profiler for games and other applications.
* [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git) - Easy to integrate Vulkan memory allocation library.


## License
Vortex is licensed with the General Public License V3
