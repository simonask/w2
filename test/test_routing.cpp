#include "wayward/app.hpp"

#include <gtest/gtest.h>

namespace w = wayward;

namespace {
    w::Request make_request(std::string path, std::map<std::string, std::string> headers = {}, std::string body = "") {
        w::Request req;
        req.method = w::Method::Get;
        req.url = std::move(path);
        req.headers = std::move(headers);
        req.body = std::move(body);
        return req;
    }

    w::Response respond(w::IRequestResponder& responder, w::Request& req) {
        w::Response res;
        responder.respond(req, res);
        return res;
    }

    w::Response respond(w::IRequestResponder& responder, const w::Request& creq) {
        w::Request req = creq;
        w::Response res;
        responder.respond(req, res);
        return res;
    }
}

TEST(Routing, Basic) {
    w::App app;
    bool get_root = false, get_foo = false;
    app.get("/", [&](auto& req, auto& res) {
        get_root = true;
    });
    app.get("/foo", [&](auto& req, auto& res) {
        get_foo = true;
    });
    respond(app, make_request("/"));
    EXPECT_TRUE(get_root);
    EXPECT_FALSE(get_foo);
    get_root = false;
    respond(app, make_request("/foo"));
    EXPECT_FALSE(get_root);
    EXPECT_TRUE(get_foo);
    get_foo = false;
    respond(app, make_request("/lol"));
    EXPECT_FALSE(get_root);
    EXPECT_FALSE(get_foo);
}

TEST(Routing, Capture) {
    w::App app;
    std::string id;
    app.get("/foo/:id", [&](auto& req, auto& res) {
        id = req.params["id"];
    });
    auto res = respond(app, make_request("/foo/123"));
    EXPECT_EQ(id, "123");
}

