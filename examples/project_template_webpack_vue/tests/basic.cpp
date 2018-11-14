#include <catch.hpp>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "env.h"

TEST_CASE("basic API functioning")
{
    auto result = cpr::Get(cpr::Url{hostname + "/api/endpoint"});
    REQUIRE(result.status_code == 200);
    REQUIRE(result.text == "null");
}

TEST_CASE("basic static asset functioning")
{
    auto result = cpr::Get(cpr::Url{hostname + "/"});
    REQUIRE(result.status_code == 200);
    const char* response = "<html>\n<div id=\"app\">\n    <nav-bar></nav-bar>\n    <div class=\"container\">\n        <router-view></router-view>\n    </div>\n</div>\n\n<script src=\"./webpack/index.js\"></script>\n</html>";
    
    REQUIRE(result.text == response);
}
