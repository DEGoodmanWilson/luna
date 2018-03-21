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


from conans import ConanFile, CMake, tools
import os

class LunaConan(ConanFile):
    name = "luna"
    version = "5.0.0"
    url = "https://github.com/DEGoodmanWilson/luna.git"
    license = "MIT"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared":   [True, False],
               "build_luna_tests":    [True, False],
               "build_luna_coverage": [True, False],
               "build_luna_examples": [True, False]}
    default_options = "shared=False", "build_luna_tests=False", "build_luna_coverage=False", "build_luna_examples=False"
    requires = "libmicrohttpd/0.9.51@DEGoodmanWilson/stable", "libmime/[~= 0.1]@DEGoodmanWilson/stable", "base64/[~= 1.0]@DEGoodmanWilson/stable"
    generators = "cmake"
    exports = ["*"] #TODO this isn't correct, we can improve this.
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
            self.requires.add("jsonformoderncpp/[~= 3.1]@vthiery/stable", private=False)
        else:
            if "nl-json" in self.requires:
                del self.requires["nl-json"]

    def build(self):
        cmake = CMake(self)
        cmake.configure(defs={
            "BUILD_LUNA_TESTS": "ON" if self.options.build_luna_tests else "OFF",
            "BUILD_LUNA_COVERAGE": "ON" if self.options.build_luna_coverage else "OFF",
            "BUILD_LUNA_EXAMPLES": "ON" if self.options.build_luna_examples else "OFF"
            })
        cmake.build()
        if(self.options.build_luna_tests):
            self.run('ctest . --verbose')
            # if(self.options.build_luna_coverage):
                # self.run("curl -s https://codecov.io/bash > codecov.sh && chmod u+x codecov.sh && ./codecov.sh /usr/bin/gcov-4.9")

    def package(self):
        self.copy(pattern="*.h", dst="include/luna", src="luna")
        self.copy(pattern="*.hpp", dst="include/luna", src="luna")
        self.copy(pattern="*.dll", dst="bin", src="bin", keep_path=False)
        self.copy(pattern="*.lib", dst="lib", src="lib", keep_path=False)
        self.copy(pattern="*.a", dst="lib", src="lib", keep_path=False)
        self.copy(pattern="*.so*", dst="lib", src="lib", keep_path=False)
        self.copy(pattern="*.dylib", dst="lib", src="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ['luna']
