//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#pragma once

#include <string>
#include <regex>
#include <map>
#include <stdint.h>


namespace luna
{

#define LUNA_FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define MAKE_STRING_LIKE(x) class x : public std::string \
{ \
public: \
    x() = default; \
    x(const x &rhs) = default; \
    x(x &&rhs) = default; \
    x &operator=(const x &rhs) = default; \
    x &operator=(x &&rhs) = default; \
    x(const char *raw_string) : std::string(raw_string) {} \
    x(const char *raw_string, size_t length) : std::string(raw_string, length) {} \
    explicit x(size_t to_fill, char character) : std::string(to_fill, character) {} \
    x(const std::string &std_string) : std::string(std_string) {} \
    x(const std::string &std_string, size_t position, size_t length = std::string::npos) \
        : std::string(std_string, position, length) {} \
    explicit x(std::initializer_list<char> il) : std::string(il) {} \
    template<class InputIterator> \
    explicit x(InputIterator first, InputIterator last) \
        : std::string(first, last) {} \
}

#define MAKE_BOOL_LIKE(x) class x \
{ \
public: \
    x() = default; \
    x(const x &rhs) = default; \
    x(x &&rhs) = default; \
    x &operator=(const x &rhs) = default; \
    x &operator=(x &&rhs) = default; \
    x(bool new_val) : value{new_val} {} \
    x & operator=(bool && new_value) {std::swap(value, new_value); return *this;} \
    x & operator=(const bool & new_value) {value = new_value; return *this;} \
    explicit operator bool() {return value;} \
private: \
    bool value; \
}

#define MAKE_UINT16_T_LIKE(x) class x\
{ \
public: \
    x() = default; \
    x(const x &rhs) = default; \
    x(x &&rhs) = default; \
    x &operator=(const x &rhs) = default; \
    x &operator=(x &&rhs) = default; \
    x(uint16_t new_val) : value{new_val} {} \
    operator uint16_t() {return value;} \
private: \
    uint16_t value; \
}

#define MAKE_INT_LIKE(T, x) class x\
{ \
public: \
    x() = default; \
    x(const x &rhs) = default; \
    x(x &&rhs) = default; \
    x &operator=(const x &rhs) = default; \
    x &operator=(x &&rhs) = default; \
    x(T new_val) : value{new_val} {} \
    operator T() {return value;} \
private: \
    T value; \
}

using status_code = uint16_t;

extern std::string default_mime_type;

using endpoint_matches = std::vector<std::string>;
using query_params = std::map<std::string, std::string>;

struct case_insensitive_comp_ {
    bool operator()(const std::string& a, const std::string& b) const noexcept;
};
using headers = std::map<std::string, std::string, case_insensitive_comp_>;
using request_headers = headers;
using response_headers = headers;

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

struct request
{
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


    response() : status_code{0}, headers{}, content_type{}, content{}, file{}
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

    response(::luna::status_code status_code, std::string content) : status_code{status_code}, content_type{default_mime_type}, content{content}, file{""}
    { }

    response(::luna::status_code status_code, const ::luna::request_headers& headers, std::string content) : status_code{status_code}, headers{headers}, content_type{default_mime_type}, content{content}, file{""}
    { }

    response(::luna::status_code status_code, std::string content_type, std::string content) :
            status_code{status_code}, content_type{content_type}, content{content}, file{""}
    { }

    response(::luna::status_code status_code, const ::luna::request_headers& headers, std::string content_type, std::string content) :
            status_code{status_code}, headers{headers}, content_type{content_type}, content{content}, file{""}
    { }

    // default success responses
    response(const ::luna::request_headers& headers, std::string content) : status_code{0}, headers{headers}, content_type{default_mime_type}, content{content}, file{""}
    { }

    response(const ::luna::request_headers& headers) : status_code{0}, headers{headers}, file{""}
    { }

    response(std::string content) : status_code{0}, content_type{default_mime_type}, content{content}, file{""}
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


} //namespace luna
