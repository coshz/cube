fn main() {
    let mut build = cc::Build::new();

    build
        .cpp(true)                 
        .std("c++17")              
        .opt_level(2)              
        .file("cpp/cube_amalg.min.cpp");

    if cfg!(target_os = "macos") {
        build.flag("-stdlib=libc++");
        println!("cargo:rustc-link-lib=c++");
    } else {
        println!("cargo:rustc-link-lib=stdc++");
    }

    build.compile("cube_cpp");

    println!("cargo:rerun-if-changed=cpp/cube_amalg.min.cpp");
}