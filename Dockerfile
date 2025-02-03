FROM archlinux:latest

RUN mkdir -p /vortex
WORKDIR /vortex

COPY . /vortex

RUN yes | pacman -Syu --noconfirm

RUN yes | pacman -S --noconfirm git cmake ninja base-devel clang vulkan-devel libx11 wayland libxinerama
RUN cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++
RUN cmake --build build

RUN ./bin/SandboxApp
