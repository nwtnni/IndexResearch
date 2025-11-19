const ROOT: &str = env!("CARGO_MANIFEST_DIR");

fn main() {
    let files = [format!("{ROOT}/cpp/wrap.cpp")];

    let includes = [
        format!("{ROOT}/include/"),
        format!("{ROOT}/../HOT/"),
        format!("{ROOT}/../HOT/include/"),
        format!("{ROOT}/../util/"),
    ];

    cxx_build::bridge("src/lib.rs")
        .cpp(true)
        .std("c++17")
        .define("AVX2_ENABLE", None)
        .files(&files)
        .includes(&includes)
        .flag("-march=native")
        .compile("hot");

    // Must be after linking
    //pkg_config::probe_library("tbb").expect("Could not find tbb");
    //pkg_config::probe_library("mimalloc").expect("Could not find mimalloc");

    for path in files.iter().chain(&includes) {
        println!("cargo:rerun-if-changed={path}");
        //println!("cargo:rerun-if-changed={ROOT}/../util/epoch.h");
    }
}
