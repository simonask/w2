#include <string>
#include <memory>

#include <wayward/http.hpp>

namespace wayward {
    struct Server {
        Server();
        ~Server();

        Server& listen(std::string listen_address, unsigned int port);
        Server& listen(std::string unix_socket_path);
        int run(IRequestResponder&);
        void stop();

    private:
        struct ClientBase;
        template <class> struct Client;
        struct AcceptorBase;
        template <class> struct Acceptor;
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
