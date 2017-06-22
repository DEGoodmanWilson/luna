from conans import ConanFile, CMake

class LunaConan(ConanFile):
    name = "luna"
    version = "3.0.0"
    url = "https://github.com/DEGoodmanWilson/luna.git"
    license = "MIT"
    settings = "os", "compiler", "build_type", "arch"
    options = {"build_shared_libs":   [True, False],
               "build_luna_tests":    [True, False],
               "build_luna_coverage": [True, False],
               "build_luna_examples": [True, False]}
    default_options = "build_shared_libs=False", "build_luna_tests=False", "build_luna_coverage=False", "build_luna_examples=False", "cpr:use_system_curl=True", "cpr:insecure_curl=True"
    requires = "libmicrohttpd/0.9.51@DEGoodmanWilson/stable", "libmagic/5.29@DEGoodmanWilson/testing", "base64/1.0.2@DEGoodmanWilson/stable"
    generators = "cmake"
    exports = ["*"]

    def configure(self):
        if self.options.build_luna_coverage:
            self.options.build_luna_tests=True

    def requirements(self):
        if self.options.build_luna_tests:
            self.requires.add("cpr/1.2.0@DEGoodmanWilson/testing", private=False)
            self.requires.add("gtest/1.7.0@lasote/stable", private=False)
            self.options["gtest"].shared = False
        else:
            if "gtest" in self.requires:
                del self.requires["gtest"]
            if "cpr" in self.requires:
                del self.requires["cpr"]

    def build(self):
        cmake = CMake(self.settings)
        build_shared_libs = "-DBUILD_SHARED_LIBS=ON" if self.options.build_shared_libs else "-DBUILD_SHARED_LIBS=OFF"
        build_luna_tests = "-DBUILD_LUNA_TESTS=ON" if self.options.build_luna_tests else "-DBUILD_LUNA_TESTS=OFF"
        build_luna_coverage = "-DBUILD_LUNA_COVERAGE=ON" if self.options.build_luna_coverage else "-DBUILD_LUNA_COVERAGE=OFF"
        build_luna_examples = "-DBUILD_LUNA_EXAMPLES=ON" if self.options.build_luna_examples else "-DBUILD_LUNA_EXAMPLES=OFF"
        self.run('cmake %s %s %s %s "%s" %s' % (build_shared_libs, build_luna_tests, build_luna_coverage, build_luna_examples, self.conanfile_directory, cmake.command_line))
        self.run('cmake --build . %s' % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include/luna", src="luna")
        self.copy("*.lib", dst="lib", src="lib")
        self.copy("*.a", dst="lib", src="lib")

    def package_info(self):
        self.cpp_info.libs = ["luna"]
