#include <catch.hpp>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "env.h"

TEST_CASE("basic API functioning")
{
    auto result = cpr::Get(cpr::Url{hostname + "/api/endpoint"});
    REQUIRE(result.status_code == 200);
    REQUIRE(result.text == "{\"made_it\": true}");
}

TEST_CASE("basic static asset functioning")
{
    auto result = cpr::Get(cpr::Url{hostname + "/"});
    REQUIRE(result.status_code == 200);
    REQUIRE(result.text == "<html>\n<h1>Hello world!</h1>\n</html>");
}
