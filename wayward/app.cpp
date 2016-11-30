#include "wayward/app.hpp"
#include <vector>

namespace wayward {
    struct App::Impl {
        std::vector<std::function<void(Request&, Response&)>> handlers;
    };

    App::App() : impl_(new Impl) {}
    App::~App() {}

    void App::get(const char* path, std::function<void(Request&, Response&)> handler) {
        // TODO: Actually handle paths
        impl_->handlers.push_back(std::move(handler));
    }

    void App::respond(Request& req, Response& res) {
        auto& handler = *impl_->handlers.begin();
        handler(req, res);
    }

    void plain_text(Response& res, std::string body) {
        res.headers["Content-Type"] = "text/plain";
        res.status = Status::OK;
        res.body = std::move(body);
    }
}
