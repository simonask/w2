#include <wayward/server.hpp>
#include <wayward/app.hpp>

namespace w = wayward;

int main(int argc, char** argv) {
    w::App app;
    app.get("/", [](const auto& req, auto& res) {
        w::plain_text(res, "Hello, Wayward!");
    });

    w::Server server;
    server.listen("::", 3000);
    return server.run(app);
}
