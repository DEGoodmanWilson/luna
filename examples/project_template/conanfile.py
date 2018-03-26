#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conans import ConanFile, CMake


class TestPackageConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = "luna/[~=5.0]@DEGoodmanWilson/stable", "jsonformoderncpp/[~= 3.1]@vthiery/stable"
    build_requires = "cpr/[~=1.3]@DEGoodmanWilson/stable", "Catch/[~=1.9]@bincrafters/stable"

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()
