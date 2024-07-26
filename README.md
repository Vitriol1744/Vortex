# Vortex Engine

[![Build](https://github.com/Vitriol1744/Vortex/actions/workflows/build.yml/badge.svg)](https://github.com/Vitriol1744/Vortex/actions/workflows/build.yml)


## Description
Low level game engine being written in modern C++23 utilizing powerfull Vulkan API

## Dependencies
To be able to use Vortex you need to install a few dependencies:

* Modern C++ compiler, e.g: clang++-18
* Vulkan SDK 
* Platform specific window server dev tools

### Setup on Arch based derivates
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
The first thing you have to do is to generate CMake Cache for the build system of your choice, e.g. Ninja like this:
```
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++-18 -DCMAKE_BUILD_TYPE=Debug
```

Afterwards you can build the engine using:
```
cmake --build build
```

The binaries are stored inside the ./bin directory

## Support
Vortex currently supports only Linux with X11 or Wayland through XWayland compatibility layer

## Roadmap
You can find the roadmap at: https://trello.com/b/NwfDrxwK/vortex-engine


## Authors and acknowledgment
* v1tr10l7

### Credits
* [fmt](https://github.com/fmtlib/fmt.git) - {fmt} is an open-source formatting library providing a fast and safe alternative to C stdio and C++ iostreams.
* [magic_enum](https://github.com/Neargye/magic_enum.git) - Header-only C++17 library provides static reflection for enums, work with any enum type without any macro or boilerplate code.
* [imgui](https://github.com/ocornut/imgui.git) - Dear ImGui is a bloat-free graphical user interface library for C++.
* [glm](https://github.com/g-truc/glm.git) - OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications.
* [glfw](https://github.com/glfw/glfw.git) - GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development.
    It provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.

## License
Vortex is licensed with the General Public License V3
