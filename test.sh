echo "compiling core (Rust)"
cargo build --release

echo "compiling test"
gcc tests/main.c -llzma -lm -I./include target/release/libruna_interpreter.a -o final/test

echo "running test"
./final/test
