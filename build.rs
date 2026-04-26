use std::env;
use std::path::PathBuf;
use std::process::Command;

fn has_lzma() -> bool {
    let test_code = r#"
        #include <lzma.h>
        int main() { return 0; }
    "#;

    let out_dir = env::var("OUT_DIR").unwrap();
    let test_file = PathBuf::from(&out_dir).join("test_lzma.c");

    std::fs::write(&test_file, test_code).unwrap();

    let output = Command::new("cc")
        .arg(&test_file)
        .arg("-llzma")
        .arg("-o")
        .arg(PathBuf::from(&out_dir).join("test_lzma"))
        .output();

    match output {
        Ok(o) => o.status.success(),
        Err(_) => false,
    }
}

fn main() {
    let has_lzma = has_lzma();

    let mut build = cc::Build::new();
    build.file("src/c/lzma.c")
        .include("src")
        .warnings(false)
        .opt_level(3);

    if has_lzma {
        println!("cargo:rustc-cfg=have_lzma");
        println!("cargo:rustc-link-lib=lzma");
        build.define("HAVE_LZMA", None);
    } else {
        println!("cargo:warning=LZMA not found, using fallback");
    }

    build.compile("runa_lzma");
}
