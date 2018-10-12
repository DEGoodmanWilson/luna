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
// Copyright © 2016–2018 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>
#include <array>
#include <thread>
#include <chrono>


TEST(file_service, serve_file_404)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/foobar.txt"});
    ASSERT_EQ(404, res.status_code);
}

namespace luna
{
std::string sanitize_path_(std::string path_to_files);
}

TEST(file_service, serve_file_malicious)
{
    luna::server server;
    auto router = server.create_router("/");

    // ** Invalid cases **
    std::string path {"../../foo/confidential.txt"};
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "");

    path = "foo/bar/../../../confidential.txt";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "");


    path = "/foo/bar/../../../confidential.txt";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "");


    path = "foo/bar/../../../confidential.txt/";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "");

    path = "/foo/bar/../../../confidential.txt/";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "");

    // ** Valid cases ** 
    path = "/foo/bar/../../test.txt";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "/test.txt");

    // check if path was cleaned
    path = "/////////foo/bar/../../test.txt";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "/////////test.txt");

    path = "foo/bar/../../test.txt/";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "test.txt/");

    path = "/foo/bar/../../test.txt/";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "/test.txt/");

    path = "/foo/bar/../test.txt";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "/foo/test.txt");

    // check if path was unchanged
    path = "foo/bar/test.txt";
    path = luna::sanitize_path_(path);
    ASSERT_TRUE(path == "foo/bar/test.txt");
}

TEST(file_service, serve_text_file)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello\n", res.text);
}

TEST(file_service, serve_text_file_2)
{
    std::string mount{"/"};
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files(mount, path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello\n", res.text);
}

TEST(file_service, serve_html_file)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html; charset=utf-8", res.header["Content-Type"]);
}

TEST(file_service, serve_binary_file)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/luna.jpg"});

    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("image/jpeg", res.header["Content-Type"]);
    ASSERT_EQ(5196, res.text.size());
}

TEST(file_service, self_serve_html_file)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test.html",
                          [=](auto req) -> luna::response
                          {
                              std::string path{STATIC_ASSET_PATH};
                              std::string full_path = path + "/tests/public/test.html";
                              return luna::response::from_file(full_path);
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html; charset=utf-8", res.header["Content-Type"]);
}

TEST(file_service, self_serve_html_file_override_mime_type)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test.html",
                          [=](auto req) -> luna::response
                          {
                              std::string path{STATIC_ASSET_PATH};
                              std::string full_path = path + "/tests/public/test.html";
                              luna::response resp = luna::response::from_file(full_path);
                              resp.content_type = "text/foobar";
                              return resp;
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/foobar", res.header["Content-Type"]);
}

TEST(file_service, css_has_its_own_mime_issues)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.css"});
    ASSERT_EQ("text/css; charset=utf-8", res.header["Content-Type"]);
}

TEST(file_service, js_has_its_own_mime_issues)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.js"});
    ASSERT_EQ("application/javascript; charset=utf-8", res.header["Content-Type"]);
}

TEST(file_service, crazy_mime_issues)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.waaat"});
    ASSERT_EQ("text/plain", res.header["Content-Type"]);
}

TEST(file_service, files_with_no_extension)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/testnoext"});
    ASSERT_EQ("text/plain", res.header["Content-Type"]);
    ASSERT_EQ("hello\n", res.text);
}

TEST(file_service, directory_with_trailing_slash_is_alias_for_index_html)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test/"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html; charset=utf-8", res.header["Content-Type"]);
    ASSERT_EQ("hello html\n", res.text);
}

TEST(file_service, directory_is_alias_for_index_html)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html; charset=utf-8", res.header["Content-Type"]);
    ASSERT_EQ("hello html\n", res.text);
}

TEST(file_service, empty_dir_with_trailing_slash_throws_404)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/empty/"});
    ASSERT_EQ(404, res.status_code);
}

TEST(file_service, empty_dir_throws_404)
{
    std::string path{STATIC_ASSET_PATH};
    luna::server server;
    auto router = server.create_router("/");
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/empty"});
    ASSERT_EQ(404, res.status_code);
}
