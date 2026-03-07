if [[ $1 -eq "x86_64" ]]; then
    ./build.sh x86_64

    echo "building test"
    gcc tests/m.c -lm -I./include final/x86_64.a -o final/test
fi

if [[ $1 -eq "aarch64" ]]; then
    ./build.sh aarch64

    echo "building test"
    clang tests/m.c -I./include final/aarch64.a -o final/test
fi

echo "running test"
./final/test
