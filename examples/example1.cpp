#include <cstdlib>
#include <string>
#include <iostream>
#include <microhttpd++/server.h>

//int print_out_key (void *cls, enum MHD_ValueKind kind,
//                   const char *key, const char *value)
//{
//    std::cout << key << ": " << value << std::endl;
//    return MHD_YES;
//}

//static int answer_to_connection(void *cls,
//                         struct MHD_Connection *connection,
//                         const char *url,
//                         const char *method,
//                         const char *version,
//                         const char *upload_data,
//                         size_t *upload_data_size,
//                         void **con_cls)
//{
//    std::cout << "New " << method << " request for " << url << " using version " << version << std::endl;
//    MHD_get_connection_values (connection, MHD_HEADER_KIND, &print_out_key, NULL);
//
//    return MHD_NO;
//}

using namespace microhttpd;

int main(void)
{
    auto port_str = std::getenv("PORT");
    const uint16_t PORT = port_str ? std::stoi(port_str) : 8888;

    server server{PORT};

    server.handle_response(request_method::GET, "/ohyeah", [](std::vector<std::string> matches, query_params params, response& response) -> status_code
        {
            std::cout << "oh yeah!" << std::endl;
            for (const auto &match : matches)
            {
                std::cout << "   " << match << std::endl;
            }

            response = {"text/json", "{\"foo\": true}"};
            return 200;
        });

    server.handle_response(request_method::GET, "^/documents/(i[0-9a-f]{6})(?:/([0-9]*))?", [](std::vector<std::string> matches, query_params params, response& response) -> status_code
        {
            std::cout << "documents" << std::endl;
            for (const auto &match : matches)
            {
                std::cout << "   " << match << std::endl;
            }

            response = {"text/json", "Yes"};
            return 200;
        });

    server.start();

//    struct MHD_Daemon *daemon;
//
//    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
//                               &answer_to_connection, NULL, MHD_OPTION_END);
//    if (NULL == daemon) return 1;
//
    while (1); //TODO HAHAHAHAHAH
//
//    MHD_stop_daemon (daemon);
//    return 0;
}