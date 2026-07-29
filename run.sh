#!/usr/bin/env bash

set -e

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$project_dir"

compiler="${CC:-cc}"
cflags=(-g -std=c99 -Wall -Wextra -pedantic)

if [[ "$(uname -s)" == "Darwin" ]]; then
    libs=(-lSDL2 -framework OpenGL -lm -lGLEW)
else
    libs=(-lSDL2 -lGL -lm -lGLU -lGLEW)
fi

mkdir -p bin
rm -f bin/skibidi-chat
"$compiler" main.c "${cflags[@]}" -o bin/skibidi-chat "${libs[@]}"

cd bin
exec ./skibidi-chat "$@"
