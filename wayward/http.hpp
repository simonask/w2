#pragma once

#include <string>
#include <map>

namespace wayward {
    enum class Status {
        OK = 200,
        InternalServerError = 500,
    };

    struct Request {
        std::string url;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    struct Response {
        Status status;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    struct IRequestResponder {
        virtual ~IRequestResponder() {}

        // Attention: This function must be thread-safe!
        virtual void respond(Request&, Response&) = 0;
    };
}
