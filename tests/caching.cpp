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
#include <iostream>
#include <chrono>

 TEST(fd_cacheing, hit_the_fd_cache)
 {
     luna::server server{luna::server::enable_internal_file_cache{true}};
     std::string path{STATIC_ASSET_PATH};
     auto router = server.create_router("/");
     router->serve_files("/", path + "/tests/public");

     server.start_async();

     // We can only test this indirectly, through speedups. This might be very unreliable.
     std::chrono::high_resolution_clock::time_point t1, t2, t3, t4;

     {
         t1 = std::chrono::high_resolution_clock::now();
         auto res = cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});
         t2 = std::chrono::high_resolution_clock::now();
         ASSERT_EQ("image/png", res.header["Content-Type"]);
         ASSERT_EQ("MISS", res.header["X-Luna-Cache"]);
     }
     auto no_cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
     std::cout << "No cacheing:   " << no_cache_duration << std::endl;

     {
         t3 = std::chrono::high_resolution_clock::now();
         auto res = cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});
         t4 = std::chrono::high_resolution_clock::now();
         ASSERT_EQ("image/png", res.header["Content-Type"]);
         ASSERT_EQ("HIT", res.header["X-Luna-Cache"]);
     }
     auto cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
     std::cout << "With cacheing: " << cache_duration << std::endl;

     ASSERT_LT(cache_duration, no_cache_duration*1.10); // allow for the underlying filesystem to cache things for us.
 }

 TEST(fd_cacheing, test_cache_timeout)
 {
     luna::server server{luna::server::enable_internal_file_cache{true},
                         luna::server::internal_file_cache_keep_alive{std::chrono::milliseconds{500}}};

     std::string path{STATIC_ASSET_PATH};
     auto router = server.create_router("/");
     router->serve_files("/", path + "/tests/public");

     server.start_async();

     // We can only test this indirectly, through speedups. This might be very unreliable.
     std::chrono::high_resolution_clock::time_point t1, t2, t3, t4;

     {
         // load the cache
         auto res = cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});
         ASSERT_EQ("MISS", res.header["X-Luna-Cache"]);
     }

     {
         // read the cached value
         t1 = std::chrono::high_resolution_clock::now();
         auto res = cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});
         t2 = std::chrono::high_resolution_clock::now();
         ASSERT_EQ("image/png", res.header["Content-Type"]);
         ASSERT_EQ("HIT", res.header["X-Luna-Cache"]);
     }
     auto cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
     std::cout << "With cacheing:   " << cache_duration << std::endl;

     std::this_thread::sleep_for(std::chrono::milliseconds{750});

     {
         // cache should be invalidated now.
         t3 = std::chrono::high_resolution_clock::now();
         auto res = cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});
         t4 = std::chrono::high_resolution_clock::now();
         ASSERT_EQ("image/png", res.header["Content-Type"]);
         ASSERT_EQ("MISS", res.header["X-Luna-Cache"]);
     }
     auto no_cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
     std::cout << "No cacheing: " << no_cache_duration << std::endl;

     ASSERT_LT(cache_duration, no_cache_duration*1.10); // allow for the underlying filesystem to cache things for us.
 }
