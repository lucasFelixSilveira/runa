if [[ $1 -eq "x86_64" ]]; then
    echo "Building for x86_64"
    ./scripts/x86_64/build.sh
fi

if [[ $1 -eq "aarch64" ]]; then
    echo "Building for aarch64"
    ./scripts/aarch64/build.sh
fi
