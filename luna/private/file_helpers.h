//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#include <string>
#include <unordered_map>

#pragma once

namespace luna
{

static const std::unordered_map <std::string, std::string> mime_types
        {
                {"html", "text/html"},
                {"htm", "text/html"},
                {"css", "text/css"},
                {"js",  "text/javascript"},
        };

static const std::vector <std::string> index_filenames
        {
                "index.html",
                "index.htm",
        };
} //namespace luna