if [[ $1 -eq "x86_64" ]]; then
    ./scripts/x86_64/build.sh
    echo "building test"
    gcc tests/m.c -lm -I./include final/x86_64.a -o final/test
fi

if [[ $1 -eq "aarch64" ]]; then
    ./scripts/aarch64/build.sh
    echo "building test"
    clang tests/m.c -I./include final/aarch64.a -o final/test
fi

echo "running test"
./final/test
