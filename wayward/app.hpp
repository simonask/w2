#pragma once

#include <functional>
#include <memory>

#include <wayward/http.hpp>
#include <wayward/util/string_view.hpp>

namespace wayward {
    struct WAYWARD_EXPORT App : IRequestResponder {
        App();
        ~App();

        void get(const char* path, std::function<void(Request&, Response&)> handler);

        // IRequestResponder
        void respond(Request&, Response&) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

    void WAYWARD_EXPORT plain_text(Response&, std::string body);
}
