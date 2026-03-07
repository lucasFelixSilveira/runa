#!/bin/bash

set -e

cd src

mkdir -p ../bin
mkdir -p ../final

objs=()

for filename in *.c; do
    if [ -f "$filename" ]; then
        obj="../bin/${filename%.c}.o"
        echo "Compiling $filename"
        clang -ggdb -c "$filename" -I../include -o "$obj"
        objs+=("$obj")
    fi
done

if [ ${#objs[@]} -eq 0 ]; then
    echo "No object files found!"
    exit 1
fi

echo "Creating static library..."
llvm-ar rcs ../final/aarch64.a "${objs[@]}"

echo "Library created at ../final/aarch64.a"
