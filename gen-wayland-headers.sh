#!/bin/sh

protocols_path=Vortex-Engine/vendor/wayland-protocols
outpath=build/Vortex-Engine

generate_header()
{
    protocol=$1
    wayland-scanner public-code < $protocols_path/$protocol.xml > $outpath/wayland-"$protocol"-client-protocol.h
}

mkdir -p build/Vortex-Engine
#wayland-scanner public-code < $protocols_path/alpha-modifier-v1.xml > $outpath/wayland-alpha-modifier-v1-client-protocol.h
#wayland-scanner private-code < $protocols_path/alpha-modifier-v1.xml > $outpath/wayland-alpha-modifier-v1-protocol.c

generate_header wayland
generate_header xdg-shell
generate_header alpha-modifier-v1
