const ROOT: &str = env!("CARGO_MANIFEST_DIR");

fn main() {
    let files = [format!("{ROOT}/cpp/wrap.cpp")];

    let includes = [
        format!("{ROOT}/include/"),
        format!("{ROOT}/../HOT/"),
        format!("{ROOT}/../HOT/include/"),
        format!("{ROOT}/../util/"),
    ];

    let mut build = cxx_build::bridge("src/lib.rs");
    build
        .cpp(true)
        .std("c++17")
        .files(&files)
        .includes(&includes)
        .flag("-march=native")
        .define("AVX2_ENABLE", None);

    let tbb_root = format!("{ROOT}/../tbb-local");
    build.include(format!("{tbb_root}/include"));

    build.compile("hot");

    println!("cargo:rustc-link-search=native={tbb_root}/build");
    println!("cargo:rustc-link-lib=tbb");

    // Must be after linking
    //pkg_config::probe_library("tbb").expect("Could not find tbb");
    //pkg_config::probe_library("mimalloc").expect("Could not find mimalloc");

    for path in files.iter().chain(&includes) {
        println!("cargo:rerun-if-changed={path}");
        //println!("cargo:rerun-if-changed={ROOT}/../util/epoch.h");
    }
}
