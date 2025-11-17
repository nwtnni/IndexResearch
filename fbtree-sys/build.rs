const ROOT: &str = env!("CARGO_MANIFEST_DIR");

fn main() {
    let files = [format!("{ROOT}/cpp/wrap.cpp")];

    let includes = [
        format!("{ROOT}/include/"),
        format!("{ROOT}/../FBTree/"),
        format!("{ROOT}/../util/"),
    ];

    cxx_build::bridge("src/lib.rs")
        .cpp(true)
        .std("c++17")
        .define("AVX2_ENABLE", None)
        .files(&files)
        .includes(&includes)
        .flag("-march=native")
        .compile("fbtree");

    // Must be after linking
    pkg_config::probe_library("tbb").expect("Could not find tbb");

    for path in files.iter().chain(&includes) {
        println!("cargo:rerun-if-changed={path}");
        println!("cargo:rerun-if-changed={ROOT}/../util/epoch.h");
    }
}
