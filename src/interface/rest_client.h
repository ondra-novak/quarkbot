#pragma once

#include <span>
#include "basic_types.h"

namespace quarkbot {

class IRestClient {
public:

    static constexpr int status_lost_connection = -1;
    static constexpr int status_unsupported_response = -2;
    static constexpr int status_system_error = -3;
    static constexpr std::string_view user_agent = "Mozilla/5.0 (compatible; quarkbot)";


    virtual ~IRestClient() = default;

    enum Method {
        GET,
        POST,
        PUT,
        DELETE,
        OPTIONS,
    };


    using HeaderLine = KeyValue;

    ///http request
    struct Request {
        ///Request identifier (optional)
        std::size_t request_id;
        ///Method of the request
        Method method = {};
        ///Path (relative to base_url, dots are not allowed)
        std::string_view path = {};
        ///optional: request body content type
        std::string_view body_content_type = {};
        ///optional: request body
        std::string_view body = {};
        ///optional: extra headers
        std::span<const HeaderLine> extra_headers = {};
        ///Timeout for the request
        unsigned int timeout_ms = 30000;

    };


    ///http response
    struct Response {
        ///This response is associated with given request id (optional)
        std::size_t request_id;
        ///status of last request
        int status;
        ///status message of last request
        std::string_view status_message;
        ///headers
        std::span<const HeaderLine> headers = {};
        ///body's content type
        std::string_view body_content_type = {};
        ///body of response
        std::string_view body = {};
        ///server time (Date field, converted to time point)
        TimeStamp server_time = {};
    };

    ///Send request
    /** You need to send request, then receive response
     *
     * @param request request to send. You can send multiple requests without waition
     * for response
     */
    virtual void send(const Request &req) noexcept;
    ///Receive response
    /**
     * @return resonse object
     *
     * @note string views in response object are valid until next receive() is called
     */
    virtual awaitable<Response> receive() noexcept;

};




}
