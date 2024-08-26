#!/bin/sh

protocols_path=Vortex-Engine/vendor/wayland-protocols
outpath=build/Vortex-Engine

mkdir -p build/Vortex-Engine
wayland-scanner public-code < $protocols_path/alpha-modifier-v1.xml > $outpath/wayland-alpha-modifier-v1-client-protocol.h
wayland-scanner private-code < $protocols_path/alpha-modifier-v1.xml > $outpath/wayland-alpha-modifier-v1-protocol.c
