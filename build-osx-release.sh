#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build/release

clang \
    -std=c99 \
    -O2 -flto \
    -DNDEBUG \
    -fno-common -ffunction-sections -fdata-sections \
    -Wall -Wextra -Wpedantic \
    -Wshadow -Wconversion -Wsign-conversion -Wvla \
    -Wstrict-prototypes -Wmissing-prototypes \
    -Wpointer-arith -Wcast-align -Wwrite-strings \
    -Wdouble-promotion -Wformat=2 \
    -Wno-unused-parameter \
    -Ivendored/raylib/headers \
    main.c \
    vendored/raylib/osx/raylib.a \
    -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGL \
    -Wl,-dead_strip \
    -o build/release/raylib-template
