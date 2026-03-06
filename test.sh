./build.sh x86_64

echo "building test"
gcc tests/m.c -lm -I./include final/x86_64.a -o final/test

echo "running test"
./final/test
