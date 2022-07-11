use cbindgen::Language::C;
use std::{
    env, fs,
    path::{Path, PathBuf},
    process,
    process::Command,
    str,
};

extern crate cbindgen;

use regex::Regex;

const MIN_PHP_API_VER: u32 = 20200930;
const MAX_PHP_API_VER: u32 = 20210902;

fn main() {
    let out_dir = env::var_os("OUT_DIR").expect("Failed to get OUT_DIR");
    let out_path = PathBuf::from(".").join("bindings_ms.rs");

    // check for docs.rs and use stub bindings if required
    if env::var("DOCS_RS").is_ok() {
        println!("cargo:warning=docs.rs detected - using stub bindings");
        println!("cargo:rustc-cfg=php_debug");
        println!("cargo:rustc-cfg=php81");

        std::fs::copy("docsrs_bindings.rs", out_path)
            .expect("Unable to copy docs.rs stub bindings to output directory.");
        return;
    }

    // use php-config to fetch includes
    let includes_cmd = Command::new("php-config")
        .arg("--includes")
        .output()
        .expect("Unable to run `php-config`. Please ensure it is visible in your PATH.");

    if !includes_cmd.status.success() {
        let stderr = String::from_utf8(includes_cmd.stderr)
            .unwrap_or_else(|_| String::from("Unable to read stderr"));
        panic!("Error running `php-config`: {}", stderr);
    }

    // Ensure the PHP API version is supported.
    // We could easily use grep and sed here but eventually we want to support
    // Windows, so it's easier to just use regex.
    let php_i_cmd = Command::new("php")
        .arg("-i")
        .output()
        .expect("Unable to run `php -i`. Please ensure it is visible in your PATH.");

    if !php_i_cmd.status.success() {
        let stderr = str::from_utf8(&includes_cmd.stderr).unwrap_or("Unable to read stderr");
        panic!("Error running `php -i`: {}", stderr);
    }

    let api_ver = Regex::new(r"PHP API => ([0-9]+)")
        .unwrap()
        .captures_iter(
            str::from_utf8(&php_i_cmd.stdout).expect("Unable to parse `php -i` stdout as UTF-8"),
        )
        .next()
        .and_then(|ver| ver.get(1))
        .and_then(|ver| ver.as_str().parse::<u32>().ok())
        .expect("Unable to retrieve PHP API version from `php -i`.");

    if !(MIN_PHP_API_VER..=MAX_PHP_API_VER).contains(&api_ver) {
        panic!("The current version of PHP is not supported. Current PHP API version: {}, requires a version between {} and {}", api_ver, MIN_PHP_API_VER, MAX_PHP_API_VER);
    }

    // Infra cfg flags - use these for things that change in the Zend API that don't
    // rely on a feature and the crate user won't care about (e.g. struct field
    // changes). Use a feature flag for an actual feature (e.g. enums being
    // introduced in PHP 8.1).
    //
    // PHP 8.0 is the baseline - no feature flags will be introduced here.
    //
    // The PHP version cfg flags should also stack - if you compile on PHP 8.2 you
    // should get both the `php81` and `php82` flags.
    const PHP_81_API_VER: u32 = 20210902;

    if api_ver >= PHP_81_API_VER {
        println!("cargo:rustc-cfg=php81");
    }

    let includes =
        String::from_utf8(includes_cmd.stdout).expect("unable to parse `php-config` stdout");

    // Build `wrapper.c` and link to Rust.
    // cc::Build::new()
    //     .file("src/wrapper.c")
    //     .includes(
    //         str::replace(includes.as_ref(), "-I", "")
    //             .split(' ')
    //             .map(Path::new),
    //     )
    //     .compile("wrapper");
// -I/Users/admin/CLionProjects/fileio/cmake-build-debug/php-src -I/Users/admin/CLionProjects/fileio/cmake-build-debug/php-src/Zend -I/Users/admin/CLionProjects/fileio/cmake-build-debug/php-src/main -I/Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk/usr/include/sys/_types/ -I/Users/admin/CLionProjects/fileio/cmake-build-debug/php-src/ext/mysqlnd -I/Users/admin/CLionProjects/fileio/cmake-build-debug/php-src/ext/mysqlnd/include -I/Users/admin/CLionProjects/fileio/cmake-build-debug/php-src/ext/mysqlnd/main -I/Users/admin/CLionProjects/fileio/cmake-build-debug/php-src/ext/mysqlnd
//     let str = String::from("-I/usr/local/Cellar/php/8.1.2/include/php -I/usr/local/Cellar/php/8.1.8/include/php/main -I/usr/local/Cellar/php/8.1.2/include/php/TSRM -I/usr/local/Cellar/php/8.1.2/include/php/Zend -I/usr/local/Cellar/php/8.1.2/include/php/ext -I/usr/local/Cellar/php/8.1.2/include/php/ext/date/lib");
    let bindgen = bindgen::Builder::default()
        .header("/Users/admin/CLionProjects/fileio/rustlib/include/mysql.h")
        .clang_args(includes.split(' '))
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .blocklist_type("size_t")
        .rustfmt_bindings(true)
        .no_copy("_zval_struct")
        .no_copy("_zend_string")
        .no_copy("_zend_string")
        .no_copy("zend_arena")
        .no_copy("_zend_array");
    //
    bindgen
        .generate()
        .expect("Unable to generate bindings for PHP")
        .write_to_file(out_path)
        .expect("Unable to write bindings file.");

    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    if !Path::new("./include").exists() {
        fs::create_dir("./include").unwrap_or_else(|e| {
            println!("{}", e.to_string());
            process::exit(-1);
        });
    }
    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_language(C)
        .with_no_includes()
        .with_sys_include("stdint.h")
        .with_sys_include("stdbool.h")
        .with_sys_include("php.h")
        .with_include("../../functions/db/db_types.h")
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("./include/stdasync_lib.h");

    let configure = Configure::get();
    println!("{}", configure.0);
    if configure.has_zts() {
        println!("cargo:rustc-cfg=php_zts");
    }

    if configure.debug() {
        println!("cargo:rustc-cfg=php_debug");
    }
}

struct Configure(String);

impl Configure {
    pub fn get() -> Self {
        let cmd = Command::new("php-config")
            .arg("--configure-options")
            .output()
            .expect("Unable to run `php-config --configure-options`. Please ensure it is visible in your PATH.");

        if !cmd.status.success() {
            let stderr = String::from_utf8(cmd.stderr)
                .unwrap_or_else(|_| String::from("Unable to read stderr"));
            panic!("Error running `php -i`: {}", stderr);
        }

        // check for the ZTS feature flag in configure
        let stdout =
            String::from_utf8(cmd.stdout).expect("Unable to read stdout from `php-config`.");
        Self(stdout)
    }

    pub fn has_zts(&self) -> bool {
        self.0.contains("--enable-zts")
    }

    pub fn debug(&self) -> bool {
        self.0.contains("--enable-debug")
    }
}
