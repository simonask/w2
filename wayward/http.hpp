#pragma once

#include <string>
#include <map>

#include <wayward/def.hpp>

namespace wayward {
    enum class Status {
        OK = 200,
        InternalServerError = 500,
    };

    struct Request {
        // TODO: Use string_view for all of this
        std::string url;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    struct Response {
        Status status;

        // TODO: Use string_view for all of this
        std::map<std::string, std::string> headers;
        std::string body;
    };

    struct WAYWARD_EXPORT IRequestResponder {
        virtual ~IRequestResponder() {}

        // Attention: This function must be thread-safe!
        virtual void respond(Request&, Response&) = 0;
    };
}
