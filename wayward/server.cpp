#include "wayward/server.hpp"
#include "wayward/util/linklist.hpp"

#include <asio.hpp>
#include <http_parser.h>

#include <iostream>
#include <sstream>

namespace wayward {
    struct Server::ClientBase {
        Server::Impl& server_impl;
        util::IntrusiveListAnchor anchor;
        asio::ip::tcp::socket socket;
        http_parser parser;

        static constexpr size_t recv_buffer_size = 1024;
        std::unique_ptr<char[]> recv_buffer;
        std::string send_buffer; // TODO

        std::string current_header_field;
        Request current_request;

        ClientBase(Impl& server_impl);
        virtual ~ClientBase() {}

        void send_response(Response);
        virtual void close() = 0;

        virtual void keep_reading() = 0;
        virtual void keep_writing() = 0;

        static int on_message_begin(http_parser*);
        static int on_headers_complete(http_parser*);
        static int on_message_complete(http_parser*);
        static int on_chunk_header(http_parser*);
        static int on_chunk_complete(http_parser*);

        static int on_url(http_parser*, const char*, size_t);
        static int on_status(http_parser*, const char*, size_t);
        static int on_header_field(http_parser*, const char*, size_t);
        static int on_header_value(http_parser*, const char*, size_t);
        static int on_body(http_parser*, const char*, size_t);

        static const http_parser_settings parser_settings;
    };

    struct Server::AcceptorBase {
        Server::Impl& server_impl;
        util::IntrusiveListAnchor anchor;

        AcceptorBase(Server::Impl& impl) : server_impl(impl) {}
        virtual ~AcceptorBase() {}

        virtual void keep_accepting() = 0;
        virtual void close() = 0;
    };

    struct Server::Impl {
        asio::io_service service;
        util::IntrusiveList<ClientBase, &ClientBase::anchor> clients;
        util::IntrusiveList<AcceptorBase, &AcceptorBase::anchor> acceptors;

        IRequestResponder* responder = nullptr;

        ~Impl();

        void keep_accepting(AcceptorBase*);
    };

    template <class Protocol>
    struct Server::Client : Server::ClientBase {
        asio::basic_stream_socket<Protocol> socket;

        Client(Server::Impl& impl) : ClientBase(impl), socket(impl.service) {}

        void close() final {
            ClientBase* dead_client = this;
            socket.cancel();
            auto handler = [dead_client]() {
                delete dead_client;
            };
            server_impl.service.post(std::move(handler));
        }

        void keep_reading() final {
            auto handler = [this](std::error_code ec, size_t len) {
                if (ec == asio::error::operation_aborted) {
                    return;
                }
                if (ec == asio::error::connection_reset) {
                    close();
                    return;
                }
                if (ec == asio::error::eof) {
                    http_parser_execute(&parser, &parser_settings, nullptr, 0);
                    if (http_should_keep_alive(&parser))
                        keep_reading();
                    else
                        close();
                    return;
                }

                if (ec) {
                    std::cerr << "socket error: " << ec.message() << "\n";
                    close();
                }
                else {
                    http_parser_execute(&parser, &parser_settings, recv_buffer.get(), len);
                }
            };
            socket.async_read_some(asio::buffer(recv_buffer.get(), recv_buffer_size), std::move(handler));
        }

        void keep_writing() final {
            auto handler = [this](std::error_code ec, size_t len) {
                if (ec == asio::error::operation_aborted) {
                    return;
                }
                if (ec == asio::error::connection_reset) {
                    close();
                    return;
                }
                if (ec == asio::error::eof) {
                    close();
                    return;
                }
                if (ec) {
                    std::cerr << "socket error while writing: " << ec.message() << "\n";
                    close();
                    return;
                }
                keep_reading();
            };
            asio::async_write(socket, asio::buffer(send_buffer.data(), send_buffer.size()), std::move(handler));
        }
    };

    template <class Protocol>
    struct Server::Acceptor : Server::AcceptorBase {
        asio::basic_socket_acceptor<Protocol> acceptor;
        Client<Protocol>* next_client = nullptr;

        template <class Endpoint>
        Acceptor(Server::Impl& impl, Endpoint endpoint) : AcceptorBase(impl), acceptor(impl.service) {
            acceptor.open(endpoint.protocol());
            acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen(10);
        }

        void close() final {
            acceptor.close();
        }

        void keep_accepting() final {
            assert(next_client == nullptr);
            next_client = new Client<Protocol>(server_impl);
            server_impl.clients.link_front(next_client);
            acceptor.async_accept(next_client->socket, [this](std::error_code ec) {
                if (ec == asio::error::operation_aborted) {
                    return;
                }
                if (ec) {
                    std::cerr << "accept(): " << ec.message() << "\n";
                    std::abort();
                }
                next_client->keep_reading();
                next_client = nullptr;
                keep_accepting();
            });
        }
    };

    const http_parser_settings Server::ClientBase::parser_settings = {
        .on_message_begin = &Server::ClientBase::on_message_begin,
        .on_url = &Server::ClientBase::on_url,
        .on_status = &Server::ClientBase::on_status,
        .on_header_field = &Server::ClientBase::on_header_field,
        .on_header_value = &Server::ClientBase::on_header_value,
        .on_headers_complete = &Server::ClientBase::on_headers_complete,
        .on_body = &Server::ClientBase::on_body,
        .on_message_complete = &Server::ClientBase::on_message_complete,
        .on_chunk_header = &Server::ClientBase::on_chunk_header,
        .on_chunk_complete = &Server::ClientBase::on_chunk_complete,
    };

    Server::Server() : impl_(new Impl) {}

    Server::~Server() {}

    Server::Impl::~Impl() {
        while (!acceptors.empty()) {
            auto it = acceptors.begin();
            AcceptorBase* acceptor = &*it;
            delete acceptor;
        }
        while (!clients.empty()) {
            auto it = clients.begin();
            ClientBase* client = &*it;
            delete client;
        }
    }

    Server& Server::listen(std::string addr, unsigned int port) {
        auto ip_addr = asio::ip::address::from_string(addr.c_str());
        asio::ip::tcp::endpoint endpoint(ip_addr, port);
        auto acceptor = new Acceptor<asio::ip::tcp>(*impl_, endpoint);
        impl_->acceptors.link_front(acceptor);
        acceptor->keep_accepting();

        std::cout << "Wayward Server listening on " << acceptor->acceptor.local_endpoint().address() << ":" << acceptor->acceptor.local_endpoint().port() << ".\n";
        return *this;
    }

    Server& Server::listen(std::string unix_socket_path) {
        asio::local::stream_protocol::endpoint endpoint(unix_socket_path);
        auto acceptor = new Acceptor<asio::local::stream_protocol>(*impl_, endpoint);
        impl_->acceptors.link_front(acceptor);
        acceptor->keep_accepting();

        std::cout << "Wayward Server listening on " << endpoint.path() <<".\n";
        return *this;
    }

    int Server::run(IRequestResponder& responder) {
        impl_->responder = &responder;
        impl_->service.run();
        return 0;
    }

    Server::ClientBase::ClientBase(Server::Impl& impl)
        : server_impl(impl)
        , socket(impl.service)
        , recv_buffer(new char[recv_buffer_size])
    {
        http_parser_init(&parser, HTTP_REQUEST);
        parser.data = this;
    }

    void Server::ClientBase::send_response(Response res) {
        std::stringstream ss;
        ss << "HTTP/1.1 " << int(res.status) << "\r\n";
        for (auto& pair: res.headers) {
            ss << pair.first << ": " << pair.second << "\r\n";
        }
        ss << "Content-Length: " << res.body.size() << "\r\n";
        ss << "\r\n";
        ss << res.body;
        send_buffer = ss.str();
        keep_writing();
    }

    int Server::ClientBase::on_message_begin(http_parser* parser) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        return 0;
    }
    int Server::ClientBase::on_headers_complete(http_parser* parser) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        return 0;
    }
    int Server::ClientBase::on_message_complete(http_parser* parser) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        Response response;
        client.server_impl.responder->respond(client.current_request, response);
        client.send_response(std::move(response));
        return 0;
    }
    int Server::ClientBase::on_chunk_header(http_parser* parser) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        return 0;
    }
    int Server::ClientBase::on_chunk_complete(http_parser* parser) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        return 0;
    }

    int Server::ClientBase::on_url(http_parser* parser, const char* url, size_t len) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        client.current_request.url = std::string(url, len);
        return 0;
    }
    int Server::ClientBase::on_status(http_parser* parser, const char*, size_t) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        return 0;
    }
    int Server::ClientBase::on_header_field(http_parser* parser, const char* field, size_t len) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        client.current_header_field = std::string(field, len);
        return 0;
    }
    int Server::ClientBase::on_header_value(http_parser* parser, const char* value, size_t len) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        client.current_request.headers[std::move(client.current_header_field)] = std::string(value, len);
        return 0;
    }
    int Server::ClientBase::on_body(http_parser* parser, const char* body, size_t len) {
        ClientBase& client = *static_cast<ClientBase*>(parser->data);
        client.current_request.body = std::string(body, len);
        return 0;
    }
}

