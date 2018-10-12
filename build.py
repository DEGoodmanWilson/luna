#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conan.packager import ConanMultiPackager
from bincrafters import build_shared

if __name__ == "__main__":
    name = build_shared.get_name_from_recipe()
    username, channel, version, login_username = build_shared.get_conan_vars()
    channel = channel or "unstable"
    is_unstable = (channel=="unstable")
    reference = "{0}/{1}".format(name, version)
    upload = "https://api.bintray.com/conan/{0}/opensource".format(username.lower())
    bincrafters = "https://api.bintray.com/conan/bincrafters/public-conan"
    vthiery = "https://api.bintray.com/conan/vthiery/conan-packages"

    builder = ConanMultiPackager(
        username=username,
        channel=channel,
        reference=reference,
        upload=upload,
        remotes=[upload, bincrafters, vthiery],
        upload_only_when_stable=is_unstable,
        # docker_entry_script='sudo apt-get -qq update && sudo apt-get -qq install -y curl',
        stable_branch_pattern="stable/*")

    builder.add_common_builds(shared_option_name=name + ":shared", pure_c=False)

    for build in builder.items:
        build.options["luna:build_luna_tests"] = True
        # TODO renable this at some point in the future
        # if os.getenv("GENERATE_COVERAGE") != None:
            # build.options["luna:build_luna_coverage"] = True
        break

    builder.run()
