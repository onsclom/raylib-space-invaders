@echo off
setlocal
cd /d "%~dp0"
if not exist build\debug mkdir build\debug

gcc ^
    -std=c99 ^
    -O0 -g3 ^
    -fno-omit-frame-pointer ^
    -DDEBUG ^
    -Wall -Wextra -Wpedantic ^
    -Wshadow -Wconversion -Wsign-conversion -Wvla ^
    -Wstrict-prototypes -Wmissing-prototypes ^
    -Wpointer-arith -Wcast-align -Wwrite-strings ^
    -Wdouble-promotion -Wformat=2 ^
    -Wno-unused-parameter ^
    -Ivendored/raylib/headers ^
    main.c ^
    vendored/raylib/windows/libraylib.a ^
    -lopengl32 -lgdi32 -lwinmm ^
    -o build/debug/raylib-template.exe
