//
//      _
//  ___/__)
// (, /      __   _
//   /   (_(_/ (_(_(_
//  (________________
//                   )
//
// Luna
// a web framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//

#include "router.h"
#include "luna/config.h"

namespace luna
{

//TODO do this better. Make this an ostream with a custom function. It's not like we haven't done that before.

#define LOG_FATAL(mesg) \
{ \
    error_log(log_level::FATAL, mesg); \
}

#define LOG_ERROR(mesg) \
{ \
    error_log(log_level::ERROR, mesg); \
}

#define LOG_INFO(mesg) \
{ \
    error_log(log_level::INFO, mesg); \
}

#define LOG_DEBUG(mesg) \
{ \
    error_log(log_level::DEBUG, mesg); \
}

void router::remove_request_handler(request_handler_handle item)
{
    //TODO this is expensive. Find a better way to store this stuff.
    //TODO validate we are receiving a valid iterator!!

    std::lock_guard<std::mutex> guard{lock_};
    request_handlers_[item.first].erase(item.second);
}

router::error_handler_handle router::handle_404(error_handler_cb callback)
{
    return handle_error(404, callback);
}

router::error_handler_handle router::handle_error(status_code code, error_handler_cb callback)
{
    std::lock_guard<std::mutex> guard{lock_};
//    return response_renderer_.handle_error(code, callback);
    // TODO
}

void router::remove_error_handler(error_handler_handle item)
{
    std::lock_guard<std::mutex> guard{lock_};
//    response_renderer_.remove_error_handler(item);
    // TODO
}

std::experimental::optional<response> router::process_request(request &request)
{
    // TODO this is here to prevent writing to the list of endpoints while we're using it. Not sure we actually need this,
    // if we can find a way to restrict writing to the list of endpoints when the server is running.
    std::unique_lock<std::mutex> ulock{lock_};

    std::cout << route_base_ << " : " << request.path << std::endl;

    // first lets validate that the path begins with our base_route_, and if it does, strip it from the request to simplify the logic below
    if (!std::regex_search(request.path, std::regex{"^" + route_base_}))
    {
        return {};
    }

    //strip the base_path_ off the reqest
    auto path = request.path.substr(route_base_.length(), std::string::npos);

    std::experimental::optional<luna::response> response;

    for (const auto &handler_tuple : request_handlers_[request.method])
    {
        std::smatch pieces_match;
        auto path_regex = std::get<std::regex>(handler_tuple);

        if (std::regex_match(path, pieces_match, path_regex))
        {
            ulock.unlock(); // found a match, can unlock as we won't continue down the list of endpoints.

            std::vector<std::string> matches;
            LOG_DEBUG(std::string{"    match: "} + path);
            for (size_t i = 0; i < pieces_match.size(); ++i)
            {
                std::ssub_match sub_match = pieces_match[i];
                std::string piece = sub_match.str();
                LOG_DEBUG(std::string{"      submatch "} + std::to_string(i) + ": " + piece);
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
                            std::string error{
                                    "Request handler for \"" + path + " is missing required parameter \"" +
                                    validator.key};
                            LOG_ERROR(error);
                            response = luna::response{400, "text/plain", error};
                            valid_params = false;
                            break; //stop examining params
                        }
                    }
                    else if (validator.required) //not present, but required
                    {
                        std::string error{
                                "Request handler for \"" + path + " is missing required parameter \"" +
                                validator.key};
                        LOG_ERROR(error);
                        response = luna::response{400, "text/plain", error};
                        valid_params = false;
                        break; //stop examining params
                    }
                }

                if (valid_params)
                {
                    //made it this far! try the callback
                    response = callback(request);
                }
            }

                // TODO there is surely a more robust way to do this;
            catch (const std::exception &e)
            {
                LOG_ERROR(std::string{"Request handler for \"" + path + "\" threw an exception: "} + e.what());
                response = luna::response{500, "text/plain", "Internal error"};
                //TODO render the stack trace, etc.
            }
            catch (...)
            {
                LOG_ERROR("Unknown internal error");
                //TODO use the same error message as above, and just log things differently and test for that.
                response = luna::response{500, "text/plain", "Unknown internal error"};
                //TODO render the stack trace, etc.
            }

            break; //exit the for loop iterating over all the request handlers
        }
    }

    return response;
}

} //namespace luna