FROM archlinux:latest

RUN mkdir -p /vortex
WORKDIR /vortex

COPY . /vortex

RUN pacman -Syu --noconfirm

RUN pacman -S --noconfirm git cmake ninja base-devel clang vulkan-devel libx11 wayland libxinerama
RUN cmake -S . -B build -G Ninja
RUN cmake --build build

RUN ./bin/SandboxApp
