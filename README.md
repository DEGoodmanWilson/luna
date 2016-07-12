[![Build Status](https://travis-ci.org/DEGoodmanWilson/luna.svg)](https://travis-ci.org/DEGoodmanWilson/luna)
[![Coverage Status](https://codecov.io/gh/DEGoodmanWilson/luna/branch/master/graph/badge.svg)](https://codecov.io/gh/DEGoodmanWilson/luna)
[![badge](https://img.shields.io/badge/conan.io-luna%2F1.0.0-green.svg?logo=data:image/png;base64%2CiVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAMAAAAolt3jAAAA1VBMVEUAAABhlctjlstkl8tlmMtlmMxlmcxmmcxnmsxpnMxpnM1qnc1sn85voM91oM11oc1xotB2oc56pNF6pNJ2ptJ8ptJ8ptN9ptN8p9N5qNJ9p9N9p9R8qtOBqdSAqtOAqtR%2BrNSCrNJ/rdWDrNWCsNWCsNaJs9eLs9iRvNuVvdyVv9yXwd2Zwt6axN6dxt%2Bfx%2BChyeGiyuGjyuCjyuGly%2BGlzOKmzOGozuKoz%2BKqz%2BOq0OOv1OWw1OWw1eWx1eWy1uay1%2Baz1%2Baz1%2Bez2Oe02Oe12ee22ujUGwH3AAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfgBQkREyOxFIh/AAAAiklEQVQI12NgAAMbOwY4sLZ2NtQ1coVKWNvoc/Eq8XDr2wB5Ig62ekza9vaOqpK2TpoMzOxaFtwqZua2Bm4makIM7OzMAjoaCqYuxooSUqJALjs7o4yVpbowvzSUy87KqSwmxQfnsrPISyFzWeWAXCkpMaBVIC4bmCsOdgiUKwh3JojLgAQ4ZCE0AMm2D29tZwe6AAAAAElFTkSuQmCC)](http://www.conan.io/source/luna/1.0.0/DEGoodmanWilson/stable)

# luna

An embedded HTTP server in idiomatic C++

-or-

A C++ wrapper for libmicrohttpd

    #include <string>
    #include <iostream>
    #include <luna/server.h>
    
    using namespace luna;
    
    int main(void)
    {
        server server{server::mime_type{"text/json"}, server::port{8443}}
    
        server.handle_request(request_method::GET, "/ohyeah", [](auto matches, auto params) -> response
            {
                return {"{\"koolade\": true}"};
            });
    
        server.handle_request(request_method::GET, "^/documents/(i[0-9a-f]{6})", [](auto matches, auto params) -> response
            {
                auto document_id = matches[1];
                return {"text/html", "<h1>Serving up document "+document_id+"</h1>"};
            });
    
        while (server); //idle while the server is running. Maybe not the best way? //TODO how to signal to server to die
    }

## TODO
  * Fix the way POSTDATA is handled so that we have a default handler, and a more sophisticated handler. Not super happy with the current flow.
  * docs docs docs
