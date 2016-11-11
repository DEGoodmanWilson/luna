//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(file_service, serve_html_file)
{
    char cwd[1024];
    getcwd(cwd, 1024);
    std::cout << cwd;
    
    luna::server server{};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}