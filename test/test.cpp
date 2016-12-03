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
    server.listen("0.0.0.0", 3001);
    // server.listen("/tmp/wayward.sock");
    return server.run(app);
}
