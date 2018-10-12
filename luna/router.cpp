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

#include "router.h"
#include "luna/private/router_impl.h"
#include "luna/config.h"
#include "server.h"
#include <mutex>
#include <vector>
#include <stack>
#include <algorithm> 

namespace luna
{

router::router(std::string route_base) : impl_{std::make_unique<router_impl>(route_base)}
{
}

// move constructor and operator
router::router(router &&) noexcept = default;
router& router::operator=(router &&) noexcept = default;

router::~router() = default;

void router::set_mime_type(std::string mime_type)
{
    impl_->mime_type_ = mime_type;
}

void router::handle_request(request_method method,
                            std::regex route,
                            router::endpoint_handler_cb callback,
                            parameter::validators validations)
{
    std::lock_guard<std::mutex> guard{impl_->lock_};
    impl_->request_handlers_[method].insert(std::end(impl_->request_handlers_[method]),
                                            std::make_tuple(
                                                    route,
                                                    callback,
                                                    validations));
}

void router::handle_request(request_method method,
                            std::string route,
                            router::endpoint_handler_cb callback,
                            parameter::validators validations)
{
    std::lock_guard<std::mutex> guard{impl_->lock_};
    impl_->request_handlers_[method].insert(std::end(impl_->request_handlers_[method]),
                                            std::make_tuple(
                                                    std::regex{route},
                                                    callback,
                                                    validations));
}

std::string router::sanitize_path(std::string path_to_files) 
{       
    std::vector<std::string> url_comps;
    std::string delimeter = "/";
    std::string final_path = "";
    std::string working_path = path_to_files;
    size_t pos = 0;
    int num_front_slash = 0, num_back_slash = 0;

    // path can contain many / as prefix or suffix
    // we want to preserve that in the final path
    while (pos < path_to_files.size() and path_to_files[pos++] == '/') {
        ++num_front_slash;
    } 

    pos = path_to_files.size() - 1;
    while (int(pos) - 1 >= 0 and path_to_files[pos--] == '/') {
        ++num_back_slash;
    }

    // Now, split by '/' to get URL components
    while ((pos = working_path.find(delimeter)) != std::string::npos) {
        std::string comp =  working_path.substr(0, pos);
        if (not comp.empty()) {
            url_comps.push_back(comp);
        }
        working_path.erase(0, pos + delimeter.length());
    }

    std::string last_comp = working_path.substr(0, pos);
    if (not last_comp.empty()) {
        url_comps.push_back(last_comp);
    }

    if (url_comps.empty()) {
        // nothing to sanitize
        return path_to_files;
    }

    std::stack<std::string> stk;
    for (auto& comp : url_comps) {
        if (comp == ".." and stk.empty()) {
            // trying to access a file outside cwd
            // Return empty string which would result in 404
            return "";
        } else if (comp == "..") {
            stk.pop();
        } else {
            stk.push(comp);
        }
    }

    // Build final path from stack
    while (not stk.empty()) {
        final_path = stk.top() + delimeter + final_path;
        stk.pop();
    }

    // remove trailing '/' added by previous stmt
    final_path.pop_back();

    // add back the /s in 
    while (num_front_slash--) {
        final_path = "/" + final_path;
    }

    while (num_back_slash--) {
        final_path += "/";
    }

    return final_path;
}

void router::serve_files(std::string mount_point, std::string path_to_files)
{   
    path_to_files = router::sanitize_path(path_to_files);
    std::regex route{mount_point + "(.*)"};
    std::string local_path{path_to_files + "/"};
    handle_request(request_method::GET, route, [=](const request &req) -> response
    {
        std::string path = local_path + req.matches[1];

        error_log(log_level::DEBUG, std::string{"File requested:  "} + req.matches[1]);
        error_log(log_level::DEBUG, std::string{"Serve from    :  "} + path);

        return response::from_file(path);
    });
}

void router::add_header(std::string &&key, std::string &&value)
{
    impl_->headers_[key] = std::move(value);
}


// Helper function to tack on headers
luna::response make_response_(luna::response &&response, luna::headers &headers_)
{
    for(const auto header : headers_)
    {
        // don't override anything already here
        if(response.headers.count(header.first) == 0)
        {
            response.headers[header.first] = header.second;
        }
    }
    return response;
}

OPT_NS::optional<luna::response> router::process_request(request &request)
{
    // TODO this is here to prevent writing to the list of endpoints while we're using it. Not sure we actually need this,
    // if we can find a way to restrict writing to the list of endpoints when the server is running.
    // we need this because our iterators can get invalidated by a concurrent insert. The insert must wait until after we are done.
    std::unique_lock<std::mutex> ulock{impl_->lock_};

    // first lets validate that the path begins with our base_route_, and if it does, strip it from the request to simplify the logic below
    if (!std::regex_search(request.path, std::regex{"^" + impl_->route_base_}))
    {
        return OPT_NS::nullopt;
    }

    //strip the base_path_ off the reqest
    auto path = request.path.substr(impl_->route_base_.length(), std::string::npos);

    OPT_NS::optional<luna::response> response;

    for (const auto &handler_tuple : impl_->request_handlers_[request.method])
    {
        std::smatch pieces_match;
        auto path_regex = std::get<std::regex>(handler_tuple);

        if (std::regex_match(path, pieces_match, path_regex))
        {
            ulock.unlock(); // found a match, can unlock as we won't continue down the list of endpoints.

            std::vector<std::string> matches;
            error_log(luna::log_level::DEBUG, std::string{"    match: "} + path);
            for (size_t i = 0; i < pieces_match.size(); ++i)
            {
                std::ssub_match sub_match = pieces_match[i];
                std::string piece = sub_match.str();
                error_log(luna::log_level::DEBUG, std::string{"      submatch "} + std::to_string(i) + ": " + piece);
                matches.emplace_back(sub_match.str());
            }

            request.matches = matches;

            auto callback = std::get<endpoint_handler_cb>(handler_tuple);
            try
            {
                // Validate the parameters passed in
                // TODO this can probably be optimized
                // TODO refactor this out!
                bool valid_params{true};
                auto validators = std::get<parameter::validators>(handler_tuple);
                for (const auto &validator : validators)
                {
                    bool present = (request.params.count(validator.key) == 0) ? false : true;
                    if (present)
                    {
                        //run the validator
                        if (!validator.validation_func(request.params[validator.key]))
                        {
                            std::string error{"Request handler for \"" + path + "\" is missing required parameter \"" + validator.key + "\""};
                            error_log(luna::log_level::ERROR, error);
                            response = make_response_({400, "text/plain", error}, impl_->headers_);
                            valid_params = false;
                            break; //stop examining params
                        }
                    }
                    else if (validator.required) //not present, but required
                    {
                        std::string error{"Request handler for \"" + path + "\" is missing required parameter \"" + validator.key + "\""};
                        error_log(luna::log_level::ERROR, error);
                        response = make_response_({400, "text/plain", error}, impl_->headers_);
                        valid_params = false;
                        break; //stop examining params
                    }
                }

                if (valid_params)
                {
                    //made it this far! try the callback
                    response = make_response_(callback(request), impl_->headers_);

                    // add mime type if needed. Don't add a mimetype for file responses
                    if (response->file.empty() && response->content_type.empty()) //no content type assigned, use the default
                    {
                        response->content_type = impl_->mime_type_;
                    }
                }
            }

                // TODO there is surely a more robust way to do this;
            catch (const std::exception &e)
            {
                error_log(luna::log_level::ERROR, std::string{"Request handler for \"" + path + "\" threw an exception: "} + e.what());
                response = make_response_({500, "text/plain", "Internal error"}, impl_->headers_);
                //TODO render the stack trace, etc.
            }
            catch (...)
            {
                error_log(luna::log_level::ERROR, "Unknown internal error");
                //TODO use the same error message as above, and just log things differently and test for that.
                response = make_response_({500, "text/plain", "Unknown internal error"}, impl_->headers_);
                //TODO render the stack trace, etc.
            }

            break; //exit the for loop iterating over all the request handlers
        }
    }

    return response;
}

} //namespace luna
