//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>

TEST(types, test_port)
{
    luna::server::port l{0};
    ASSERT_EQ(0, static_cast<uint16_t>(l));
}

TEST(types, test_mime_type)
{
    luna::server::mime_type l{"hello"};
    ASSERT_EQ("hello", l);
}

TEST(types, test_connection_memory_limit)
{
    luna::server::connection_memory_limit l{0};
    ASSERT_EQ(0, static_cast<uint32_t>(l));
}

TEST(types, test_connection_limit)
{
    luna::server::connection_limit l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_connection_timeout)
{
    luna::server::connection_timeout l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_per_ip_connection_limit)
{
    luna::server::per_ip_connection_limit l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_https_mem_key)
{
    luna::server::https_mem_key l{"hello"};
    ASSERT_EQ("hello", l);
}

TEST(types, test_https_mem_cert)
{
    luna::server::https_mem_cert l{"hello"};
    ASSERT_EQ("hello", l);
}

TEST(types, test_https_priorities)
{
    luna::server::https_priorities l{"hello"};
    ASSERT_EQ("hello", l);
}

TEST(types, test_listen_socket)
{
    luna::server::listen_socket l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_thread_pool_size)
{
    luna::server::thread_pool_size l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_nonce_nc_size)
{
    luna::server::nonce_nc_size l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_thread_stack_size)
{
    luna::server::thread_stack_size l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_https_mem_trust)
{
    luna::server::https_mem_trust l{"hello"};
    ASSERT_EQ("hello", l);
}

TEST(types, test_connection_memory_increment)
{
    luna::server::connection_memory_increment l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_tcp_fastopen_queue_size)
{
    luna::server::tcp_fastopen_queue_size l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_https_mem_dhparams)
{
    luna::server::https_mem_dhparams l{"hello"};
    ASSERT_EQ("hello", l);
}

TEST(types, test_listening_address_reuse)
{
    luna::server::listening_address_reuse l{0};
    ASSERT_EQ(0, static_cast<int64_t>(l));
}

TEST(types, test_https_key_password)
{
    luna::server::https_key_password l{"hello"};
    ASSERT_EQ("hello", l);
}

TEST(types, test_header_type_case_insensitivity)
{
    luna::headers header{{"abc", "xyz"}};
    ASSERT_EQ(1, header.count("abc"));
    ASSERT_EQ(1, header.count("aBc"));
    ASSERT_EQ(1, header.count("ABC"));
    ASSERT_EQ(0, header.count("lmn"));
}