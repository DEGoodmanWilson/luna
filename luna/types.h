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

#pragma once

#include <string>
#include <regex>
#include <map>
#include <chrono>
#include <functional>
#include <stdint.h>


namespace luna
{

#define LUNA_FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define MAKE_LIKE(T, x) struct x \
{ \
    x(T s) \
            : value_(s) \
    {} \
    T get() const { return value_; } \
    T value_; \
    operator T() const { return value_; } \
};

using status_code = uint16_t;

using endpoint_matches = std::vector<std::string>;

using case_sensitive_map = std::map<std::string, std::string>;

struct case_insensitive_comp_ {
    bool operator()(const std::string& a, const std::string& b) const noexcept;
};
using case_insensitive_map = std::map<std::string, std::string, case_insensitive_comp_>;

using query_params = case_sensitive_map;

using headers = case_insensitive_map;
using request_headers = case_insensitive_map;
using response_headers = case_insensitive_map;

struct basic_authorization
{
    std::string username;
    std::string password;
    explicit operator bool() {return present_;}
    basic_authorization(bool present) : present_{present} {}
    basic_authorization(bool present, const std::string &user, const std::string &pass) : present_{present}, username{user}, password{pass} {}
private:
    bool present_;
};

basic_authorization get_basic_authorization(const request_headers &headers);


enum class request_method
{
    UNKNOWN = 0,
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    OPTIONS,
    //Yes, there are more than these. Later, though. Later.
    //HEAD,
};

std::string to_string(const luna::request_method method);

struct request
{
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    std::string ip_address;
    request_method method;
    std::string path;
    std::string http_version;
    endpoint_matches matches;
    query_params params;
    request_headers headers;
    std::string body;
};


struct response
{
    luna::status_code status_code;
    response_headers headers;
    std::string content_type;
    std::string content;
    std::string file;

    struct URI
    {
        URI() = default;
        URI(std::string location) : uri{location} {}
        std::string uri;
    };


    response() : status_code{404}, headers{}, content_type{}, content{}, file{}
    { }

    static response from_file(std::string filename)
    {
        response r;
        r.file = filename;
        return r;
    }

    // explicit status code responses
    // TODO this is now officially messy. Let's use some variadic templates to clean this up. Later.

    response(::luna::status_code status_code) : status_code{status_code}, file{""}
    { }

    response(::luna::status_code status_code, const ::luna::request_headers& headers) : status_code{status_code}, headers{headers}, file{""}
    { }

    response(::luna::status_code status_code, std::string content) : status_code{status_code}, content{content}, file{""}
    { }

    response(::luna::status_code status_code, const ::luna::request_headers& headers, std::string content) : status_code{status_code}, headers{headers}, content{content}, file{""}
    { }

    response(::luna::status_code status_code, std::string content_type, std::string content) :
            status_code{status_code}, content_type{content_type}, content{content}, file{""}
    { }

    response(::luna::status_code status_code, const ::luna::request_headers& headers, std::string content_type, std::string content) :
            status_code{status_code}, headers{headers}, content_type{content_type}, content{content}, file{""}
    { }

    // default success responses
    response(const ::luna::request_headers& headers, std::string content) : status_code{0}, headers{headers}, content{content}, file{""}
    { }

    response(const ::luna::request_headers& headers) : status_code{0}, headers{headers}, file{""}
    { }

    response(std::string content) : status_code{0}, content{content}, file{""}
    { }

    response(const ::luna::request_headers& headers, std::string content_type, std::string content) :
            status_code{0}, headers{headers}, content_type{content_type}, content{content}, file{""}
    { }

    response(std::string content_type, std::string content) :
            status_code{0}, content_type{content_type}, content{content}, file{""}
    { }

    // responses with redirects
    response(URI redirect) :
            status_code{301}, headers{{"Location", redirect.uri}}, file{""}
    { }

    response(::luna::status_code status_code, URI redirect) :
            status_code{status_code}, headers{{"Location", redirect.uri}}, file{""}
    { }
};

enum class authorization_kind
{
    BASIC =0,
};

std::string to_string(const authorization_kind kind);

struct unauthorized_response : public response
{
    unauthorized_response(const std::string &realm, const authorization_kind kind = authorization_kind::BASIC) : response{401, {{"WWW-Authenticate", to_string(kind) + " realm=\"" + realm + "\""}}} {}
};


namespace parameter
{

// default validators
auto any = [](const std::string &a) -> bool
{
    return true;
};

auto match = [](const std::string &a, const std::string &b) -> bool
{
    return a == b;
};

auto number = [](const std::string &a) -> bool
{
    return std::regex_match(a, std::regex{"\\d+"});
};

auto regex = [](const std::string &a, const std::regex &r) -> bool
{
    return std::regex_search(a, r);
};

auto validate = [](auto validator, auto ...rest)
{
    return [=](std::string to_validate) -> bool
    {
        return validator(to_validate, rest...);
    };
};

const bool optional = false;
const bool required = true;

using validation_function = std::function<bool(std::string)>;

struct validator
{
    std::string key;
    bool required;
    validation_function validation_func;

    validator(std::string &&key, bool required, validation_function validation_func=any) : key{std::move(key)}, required{required}, validation_func{validation_func} {};
    validator(const std::string &key, bool required, validation_function validation_func=any) : key{key}, required{required}, validation_func{validation_func} {};

};

using validators = std::vector<validator>;

} //namespace parameter



//namespace middleware
//{
//
//using before_request_handler_func = std::function<void(request &)>;
//struct before_request_handler
//{
//    std::vector<before_request_handler_func> funcs;
//    before_request_handler() = default;
//    before_request_handler(std::vector<before_request_handler_func> f) : funcs{f} {}
//    before_request_handler(std::initializer_list<before_request_handler_func> &&l) : funcs{std::move(l)} {}
//};
//
//using after_request_handler_func = std::function<void(response &)>;
//struct after_request_handler
//{
//    std::vector<after_request_handler_func> funcs;
//    after_request_handler() = default;
//    after_request_handler(std::vector<after_request_handler_func> f) : funcs{f} {}
//    after_request_handler(std::initializer_list<after_request_handler_func> &&l) : funcs{std::move(l)} {}
//};
//
//using after_error_func = std::function<void(response &)>;
//struct after_error
//{
//    std::vector<after_error_func> funcs;
//    after_error() = default;
//    after_error(std::vector<after_error_func> f) : funcs{f} {}
//    after_error(std::initializer_list<after_error_func> &&l) : funcs{std::move(l)} {}
//};
//
//} //namespace middleware

namespace cache
{
using read = std::function<std::shared_ptr<std::string>(const std::string &key)>; //yes, std::stirng can contain binary data! It's just a wrapper around a char array after all
using write = std::function<void(const std::string &key, std::shared_ptr<std::string> value)>; //you are going to need to copy the data out of value, it's ok. We can maybe speed this up later.
template<typename T, typename U>
std::pair<read,write> build(T t, U u)
{
    return std::make_pair(t,u);
};
}

} //namespace luna
