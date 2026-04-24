#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build/debug

clang \
    -std=c99 \
    -O0 -g3 \
    -fno-omit-frame-pointer \
    -fsanitize=address,undefined \
    -DDEBUG \
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
    -o build/debug/raylib-template
