// build.rs
use std::env;
use std::path::PathBuf;

fn main() {
    cc::Build::new()
        .file("src/c/lzma.c")
        .include("src")
        .flag("-llzma")
        .warnings(false)
        .opt_level(3)
        .compile("runa_lzma");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    let bindings = bindgen::Builder::default()
        .header("src/c/lzma.h")
        .allowlist_function("runa_compress")
        .allowlist_function("runa_decompress")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()
        .expect("Unable to generate bindings");

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");

    println!("cargo::rerun-if-changed=src/c/lzma.c");
    println!("cargo::rerun-if-changed=src/c/lzma.h");
}
