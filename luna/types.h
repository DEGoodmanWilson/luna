//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#pragma once

#include <string>
#include <regex>
#include <map>


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
    x & operator=(bool && new_value) {value = new_value;} \
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

struct response
{
    status_code status_code;
    std::string content_type;
    std::string content;

    struct URI
    {
        URI() = default;
        URI(std::string location) : uri{location} {}
        std::string uri;
    };

    URI redirect;

    response() = default;

    // explicit status code responses
    response(::luna::status_code status_code) : status_code{status_code}
    { }

    response(::luna::status_code status_code, std::string content) : status_code{status_code}, content{content}
    { }

    response(::luna::status_code status_code, std::string content_type, std::string content) :
            status_code{status_code}, content_type{content_type}, content{content}
    { }

    // default success responses
    response(std::string content) : status_code{0}, content_type{default_mime_type}, content{content}
    { }

    response(std::string content_type, std::string content) :
            status_code{0}, content_type{content_type}, content{content}
    { }

    // responses with redirects
    response(URI redirect) :
            status_code{301}, redirect{redirect}
    { }

    response(::luna::status_code status_code, URI redirect) :
            status_code{status_code}, redirect{redirect}
    { }
};

enum class request_method
{
    UNKNOWN = 0,
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    //Yes, there are more than these. Later, though. Later.
    //HEAD,
    //OPTIONS,
};

using endpoint_matches = std::vector<std::string>;
using query_params = std::map<std::string, std::string>;
using headers = std::map<std::string, std::string>;
} //namespace luna
