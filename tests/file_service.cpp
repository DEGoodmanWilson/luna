//
//      _
//  ___/_)
// (, /      ,_   _
//   /   (_(_/ (_(_(_
// CX________________
//                   )
//
// Luna
// A web application and API framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>
#include <array>
#include <thread>
#include <chrono>


TEST(file_service, serve_file_404)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/foobar.txt"});
    ASSERT_EQ(404, res.status_code);
}

TEST(file_service, serve_text_file)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(file_service, serve_text_file_2)
{
    std::string mount{"/"};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files(mount, path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(file_service, serve_html_file)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html", res.header["Content-Type"]);
}

TEST(file_service, serve_binary_file)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/luna.jpg"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("image/jpeg; charset=binary", res.header["Content-Type"]);
    ASSERT_EQ(5196, res.text.size());
}

TEST(file_service, self_serve_html_file)
{
    luna::router router{"/"};
    router.handle_request(luna::request_method::GET,
                          "/test.html",
                          [=](auto req) -> luna::response
                          {
                              std::string path{std::getenv("STATIC_ASSET_PATH")};
                              std::string full_path = path + "/tests/public/test.html";
                              return luna::response::from_file(full_path);
                          });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html", res.header["Content-Type"]);
}

TEST(file_service, self_serve_html_file_override_mime_type)
{
    luna::router router{"/"};
    router.handle_request(luna::request_method::GET,
                          "/test.html",
                          [=](auto req) -> luna::response
                          {
                              std::string path{std::getenv("STATIC_ASSET_PATH")};
                              std::string full_path = path + "/tests/public/test.html";
                              luna::response resp = luna::response::from_file(full_path);
                              resp.content_type = "text/foobar";
                              return resp;
                          });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/foobar", res.header["Content-Type"]);
}

TEST(file_service, css_has_its_own_mime_issues)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.css"});
    ASSERT_EQ("text/css", res.header["Content-Type"]);
}

TEST(file_service, js_has_its_own_mime_issues)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.js"});
    ASSERT_EQ("text/javascript", res.header["Content-Type"]);
}

TEST(file_service, directory_with_trailing_slash_is_alias_for_index_html)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test/"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html", res.header["Content-Type"]);
    ASSERT_EQ("hello html", res.text);
}

TEST(file_service, directory_is_alias_for_index_html)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html", res.header["Content-Type"]);
    ASSERT_EQ("hello html", res.text);
}

TEST(file_service, empty_dir_with_trailing_slash_throws_404)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/empty/"});
    ASSERT_EQ(404, res.status_code);
}

TEST(file_service, empty_dir_throws_404)
{
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    luna::router router{"/"};
    router.serve_files("/", path + "/tests/public");

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/empty"});
    ASSERT_EQ(404, res.status_code);
}