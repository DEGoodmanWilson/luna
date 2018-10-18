#include "logger.h"

// Keep it simple for now
#include <iostream>

void error_logger(luna::log_level level, const std::string &message)
{
    switch (level)
    {
        case luna::log_level::DEBUG:
            std::cerr << "[  DEBUG] " << message << std::endl;
            break;
        case luna::log_level::INFO:
            std::cerr << "[   INFO] " << message << std::endl;;
            break;
        case luna::log_level::WARNING:
            std::cerr << "[WARNING] " << message << std::endl;;
            break;
        case luna::log_level::ERROR:
            std::cerr << "[  ERROR] " << message << std::endl;;
            break;
        case luna::log_level::FATAL:
            std::cerr << "[  FATAL] " << message << std::endl;;
            break;
    }
}

void access_logger(const luna::request &request, const luna::response &response)
{
    std::cout << request.ip_address << ": " << luna::to_string(request.method) << " [" << response.status_code << "] "
              << request.path << " " << request.http_version << " " << (request.headers.count("user-agent") ? request.headers.at("user-agent") : "[no user-agent]") << " { "
              << std::chrono::duration_cast<std::chrono::microseconds>(request.end - request.start).count() << "us } " << std::endl;
}
