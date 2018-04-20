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

#include "types.h"
#include <regex>
#include <base64/base64.h>

namespace luna
{



std::string to_string(const authorization_kind kind)
{
    switch (kind)
    {
        case authorization_kind::BASIC:
            return "Basic";
        default:
            return "";
    }
}

std::string to_string(const request_method method)
{
    switch (method)
    {
        case request_method::UNKNOWN:
            return "UNKNOWN";
        case request_method::GET:
            return "GET";
        case request_method::POST:
            return "POST";
        case request_method::PUT:
            return "PUT";
        case request_method::PATCH:
            return "PATCH";
        case request_method::DELETE:
            return "DELETE";
        case request_method::OPTIONS:
            return "OPTIONS";
        default:
            return "UNKNOWN";
    }
}

bool case_insensitive_comp_::operator()(const std::string &a, const std::string &b) const noexcept
{
    return strcasecmp(a.c_str(), b.c_str()) < 0;
}

basic_authorization get_basic_authorization(const request_headers &headers)
{
    // First, find the headers
    if (0 == headers.count("Authorization"))
    {
        return {false};
    }

    // Ensure that the header is of the form "Basic abc", and extract the encoded bit
    std::regex basic_regex(R"(Basic ([a-zA-Z0-9\+\/=]+))"); // We should look into also accepting RFC 4648
    std::smatch basic_match;
    if (!std::regex_match(headers.at("Authorization"), basic_match, basic_regex) || (basic_match.size() != 2))
    {
        return {false};
    }

    // The first sub_match is the whole string; the next
    // sub_match is the first parenthesized expression.
    std::string userpass = base64_decode(basic_match[1].str());

    // We have a string of the form, probably, of username:password. Let's extract the username and password
    std::regex userpass_regex("(.*?):(.*)");
    std::smatch userpass_match;
    if (!std::regex_match(userpass, userpass_match, userpass_regex) || (userpass_match.size() != 3))
    {
        return {false};
    }

    // extract username and password

    return {true, userpass_match[1].str(), userpass_match[2].str()};
}

} //namespace luna