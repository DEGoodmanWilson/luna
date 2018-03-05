# coding: utf-8
#
#       _
#   ___/_)
#  (, /      ,_   _
#    /   (_(_/ (_(_(_
#  CX________________
#                    )
#
#  Luna
#  A web application and API framework in modern C++
#
#  Copyright © 2016-2017 D.E. Goodman-Wilson
#


from conans import ConanFile, CMake

class LunaConan(ConanFile):
    name = "luna"
    version = "4.0.2"
    url = "https://github.com/DEGoodmanWilson/luna.git"
    license = "MIT"
    settings = "os", "compiler", "build_type", "arch"
    options = {"build_shared_libs":   [True, False],
               "build_luna_tests":    [True, False],
               "build_luna_coverage": [True, False],
               "build_luna_examples": [True, False]}
    default_options = "build_shared_libs=False", "build_luna_tests=False", "build_luna_coverage=False", "build_luna_examples=False"
    # requires = "libmicrohttpd/0.9.51@DEGoodmanWilson/stable", "libmagic/5.25@DEGoodmanWilson/stable", "base64/1.0.2@DEGoodmanWilson/stable"
    requires = "libmicrohttpd/0.9.51@DEGoodmanWilson/stable", "base64/1.0.2@DEGoodmanWilson/stable"
    generators = "cmake"
    exports = ["*"]
    description = "A web application and API framework in modern C++"

    def configure(self):
        if self.options.build_luna_coverage:
            self.options.build_luna_tests=True

    def requirements(self):
        if self.options.build_luna_tests:
            self.requires.add("cpr/1.3.0@DEGoodmanWilson/stable", private=False)
            self.requires.add("gtest/1.8.0@bincrafters/stable", private=False)
            self.options["gtest"].shared = False
        else:
            if "gtest" in self.requires:
                del self.requires["gtest"]
            if "cpr" in self.requires:
                del self.requires["cpr"]

        if self.options.build_luna_examples:
            self.requires.add("nl-json/2.1.1@genvidtech/1.4.0", private=False)
        else:
            if "nl-json" in self.requires:
                del self.requires["nl-json"]

    def build(self):
        cmake = CMake(self)
        build_shared_libs = "-DBUILD_SHARED_LIBS=ON" if self.options.build_shared_libs else "-DBUILD_SHARED_LIBS=OFF"
        build_luna_tests = "-DBUILD_LUNA_TESTS=ON" if self.options.build_luna_tests else "-DBUILD_LUNA_TESTS=OFF"
        build_luna_coverage = "-DBUILD_LUNA_COVERAGE=ON" if self.options.build_luna_coverage else "-DBUILD_LUNA_COVERAGE=OFF"
        build_luna_examples = "-DBUILD_LUNA_EXAMPLES=ON" if self.options.build_luna_examples else "-DBUILD_LUNA_EXAMPLES=OFF"
        self.run('cmake %s %s %s %s "%s" %s' % (build_shared_libs, build_luna_tests, build_luna_coverage, build_luna_examples, self.source_folder, cmake.command_line))
        self.run('cmake --build . %s' % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include/luna", src="luna")
        self.copy("*.hpp", dst="include/luna", src="luna")
        self.copy("*.lib", dst="lib", src="lib")
        self.copy("*.a", dst="lib", src="lib")

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        seld.cpp_info.libs.append("magic") # sigh
