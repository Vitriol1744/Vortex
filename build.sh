#!/usr/bin/env bash
#
# Created by v1tr10l7 on 15.11.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
# Builds Vortex for the host (CMake + Ninja) or for Android (Gradle).
#
#   ./build.sh                      host, Debug, into build/
#   ./build.sh release              host, Release, into build-release/
#   ./build.sh --run                build, then run Nexus
#   ./build.sh android              Android debug APK (gradlew assembleDebug)
#   ./build.sh android release      Android release APK
#   ./build.sh android --install    build and install on a connected device
#   ./build.sh --clean ...          wipe the build directory first
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
    sed -n '/^# Builds/,/^set -euo/p' "${BASH_SOURCE[0]}" | sed '$d; s/^# \{0,1\}//'
    cat <<'EOF'
Options:
  host | android          platform (default: host)
  debug | release | relwithdebinfo | minsizerel
                          configuration (default: debug)
  -c, --clean             remove the build directory before building
  -r, --run               host: run Nexus after building
  -i, --install           android: install the APK on a connected device
  -j, --jobs N            parallel jobs (host default: by RAM, ~1.5GB a job;
                          android default: gradle's own choice)
  -t, --target NAME       host: build only this CMake target (e.g. Nexus)
  -h, --help              show this help
Anything after `--` is passed to the underlying build tool.
EOF
}

platform=host
config=debug
clean=0
run=0
install=0
jobs=""
target=""
passthrough=()

while (($#)); do
    case "$1" in
        host | linux | windows | android) platform="$1" ;;
        debug | release | relwithdebinfo | minsizerel) config="$1" ;;
        -c | --clean) clean=1 ;;
        -r | --run) run=1 ;;
        -i | --install) install=1 ;;
        -j | --jobs) jobs="${2:?--jobs needs a number}"; shift ;;
        -j*) jobs="${1#-j}" ;;
        -t | --target) target="${2:?--target needs a name}"; shift ;;
        -h | --help) usage; exit 0 ;;
        --) shift; passthrough=("$@"); break ;;
        *) echo "build.sh: unknown argument '$1' (see --help)" >&2; exit 2 ;;
    esac
    shift
done
[[ "$platform" == linux || "$platform" == windows ]] && platform=host

log() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }

build_android() {
    local project="$ROOT/Vortex-Android"
    local variant
    variant="$(tr '[:lower:]' '[:upper:]' <<<"${config:0:1}")${config:1}"
    # Gradle only has Debug and Release variants.
    [[ "$variant" == Debug || "$variant" == Release ]] || variant=Release

    local task="assemble$variant"
    ((install)) && task="install$variant"

    local args=("$task")
    ((clean)) && args=(clean "${args[@]}")
    [[ -n "$jobs" ]] && args+=("--max-workers=$jobs")

    log "Android: ./gradlew ${args[*]} ${passthrough[*]}"
    cd "$project"
    ./gradlew "${args[@]}" "${passthrough[@]}"
}

build_host() {
    local build_type
    case "$config" in
        debug) build_type=Debug ;;
        release) build_type=Release ;;
        relwithdebinfo) build_type=RelWithDebInfo ;;
        minsizerel) build_type=MinSizeRel ;;
    esac
    # Debug keeps the existing build/ directory; other configs get their own.
    local build_dir="$ROOT/build"
    [[ "$config" != debug ]] && build_dir="$ROOT/build-$config"

    if ((clean)) && [[ -d "$build_dir" ]]; then
        log "Removing $build_dir"
        rm -rf "$build_dir"
    fi

    # Configure only when needed: the first time, or if the cache was lost.
    if [[ ! -f "$build_dir/CMakeCache.txt" ]]; then
        local configure=(-S "$ROOT" -B "$build_dir" -G Ninja
            -DCMAKE_BUILD_TYPE="$build_type")
        # Vortex's C++20 modules are built and tested with clang.
        if command -v clang++ >/dev/null; then
            configure+=(-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++)
        fi
        if command -v ccache >/dev/null; then
            configure+=(-DCMAKE_C_COMPILER_LAUNCHER=ccache
                -DCMAKE_CXX_COMPILER_LAUNCHER=ccache)
        fi
        log "Configuring ($build_type): cmake ${configure[*]}"
        cmake "${configure[@]}"
    fi

    # Ninja's default (cores + 2) runs out of memory here: a clang job that
    # pulls in vulkan.hpp and Prism's BMIs, under ASan, can take ~1.5GB. Cap
    # the jobs by RAM unless the caller chose a number.
    if [[ -z "$jobs" && -r /proc/meminfo ]]; then
        local mem_kb cores
        mem_kb=$(awk '/^MemTotal:/ { print $2 }' /proc/meminfo)
        cores=$(nproc)
        jobs=$((mem_kb / (1536 * 1024)))
        ((jobs < 1)) && jobs=1
        ((jobs > cores)) && jobs=$cores
    fi

    local args=(--build "$build_dir")
    [[ -n "$target" ]] && args+=(--target "$target")
    [[ -n "$jobs" ]] && args+=(--parallel "$jobs")

    log "Building ($build_type): cmake ${args[*]} ${passthrough[*]}"
    if ((${#passthrough[@]})); then
        cmake "${args[@]}" -- "${passthrough[@]}"
    else
        cmake "${args[@]}"
    fi

    if ((run)); then
        # Nexus loads assets/ and editorPreferences.json relative to the cwd.
        # Debug builds go to bin/, the others to bin/<Config>/.
        local nexus=./bin/Nexus
        [[ "$build_type" != Debug ]] && nexus="./bin/$build_type/Nexus"
        log "Running $nexus"
        cd "$ROOT"
        exec "$nexus"
    fi
}

case "$platform" in
    android) build_android ;;
    host) build_host ;;
esac
