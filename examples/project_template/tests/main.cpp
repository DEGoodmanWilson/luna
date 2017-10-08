#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

std::string hostname{"http://localhost"};

int main( int argc, char* argv[] )
{
    if (auto port = std::getenv("PORT"))
    {
        hostname = hostname + ":" + port;
    }
    else
    {
        hostname = hostname + ":8080";
    }

    int result = Catch::Session().run( argc, argv );

    return ( result < 0xff ? result : 0xff );
}