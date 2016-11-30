#include <string>
#include <memory>

#include <wayward/http.hpp>

namespace wayward {
    struct Server {
        Server();
        ~Server();

        Server& listen(std::string listen_address, unsigned int port);
        int run(IRequestResponder&);
        void stop();

    private:
        struct Client;
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
