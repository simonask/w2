#include "wayward/app.hpp"
#include <vector>
#include <cassert>

using namespace wayward::util;

namespace {
    using Path = std::vector<StringView>;

    Path split_path_components(StringView path) {
        Path result;
        size_t p = 0;
        while (true) {
            size_t q = path.find('/', p);
            if (q == std::string::npos) {
                result.emplace_back(path.data() + p, path.size() - p);
                break;
            } else {
                StringView component{path.data() + p, p - q};
                if (component.size() != 0) {
                    result.push_back(component);
                }
                p = q + 1;
            }
        }
        return result;
    }

    struct IPathComponentMatcher {
        virtual ~IPathComponentMatcher() {}
        virtual bool match(StringView component) const = 0;
        virtual void apply(StringView component, wayward::Request& req) const = 0;
    };

    // Handle ':placeholder' in paths
    struct PlaceholderMatcher : IPathComponentMatcher {
        std::string name;
        explicit PlaceholderMatcher(StringView name) : name(name) {}

        bool match(StringView component) const override final {
            return true;
        }

        void apply(StringView component, wayward::Request& req) const override final {
            req.params[name] = component;
        }
    };

    // Handle literal path components
    struct ExactMatcher : IPathComponentMatcher {
        std::string component;
        explicit ExactMatcher(std::string component) : component(std::move(component)) {}

        bool match(StringView c) const override final {
            return component == c;
        }

        void apply(StringView, wayward::Request&) const override final {}
    };

    struct PathMatcher {
        PathMatcher() {}
        PathMatcher(PathMatcher&&) noexcept = default;
        PathMatcher(const PathMatcher&) = delete;
        std::vector<std::unique_ptr<IPathComponentMatcher>> component_matchers;

        bool match(Path components) const {
            if (components.size() != component_matchers.size())
                return false;

            for (size_t i = 0; i < components.size(); ++i) {
                if (!component_matchers[i]->match(components[i]))
                    return false;
            }
            return true;
        }

        void apply(Path components, wayward::Request& req) const {
            assert(components.size() == component_matchers.size());
            for (size_t i = 0; i < components.size(); ++i) {
                component_matchers[i]->apply(components[i], req);
            }
        }
    };
}

namespace wayward {

    struct App::Impl {
        using PathMatchers = std::vector<std::tuple<PathMatcher, std::function<Handler>>>;
        std::map<Method, PathMatchers> path_matchers;

        void install_handler(Method method, StringView path, std::function<Handler> handler) {
            auto it = path_matchers.find(method);
            if (it != path_matchers.end()) {
                it = path_matchers.insert(std::make_pair(method, PathMatchers{})).first;
            }
            PathMatchers& matchers = it->second;

            PathMatcher matcher;
            Path matcher_components = split_path_components(path);
            for (auto& matcher_component: matcher_components) {
                std::unique_ptr<IPathComponentMatcher> m;
                if (matcher_component[0] == ':') {
                    m = std::make_unique<PlaceholderMatcher>(matcher_component.substr(1));
                } else {
                    m = std::make_unique<ExactMatcher>(matcher_component);
                }
                matcher.component_matchers.push_back(std::move(m));
            }

            matchers.emplace_back(std::move(matcher), std::move(handler));
        }
    };

    App::App() : impl_(new Impl) {}
    App::~App() {}

    void App::get(const char* cpath, std::function<void(Request&, Response&)> handler) {
        impl_->install_handler(Method::Get, cpath, std::move(handler));
    }

    void App::respond(Request& req, Response& res) {
        Path components = split_path_components(req.url);
        auto it = impl_->path_matchers.find(req.method);
        if (it == impl_->path_matchers.end()) {
            throw std::runtime_error{"404"}; // TODO
        }
        for (auto& matcher: it->second) {
            PathMatcher& m = std::get<0>(matcher);
            std::function<Handler>& handler = std::get<1>(matcher);
            if (m.match(components)) {
                m.apply(components, req);
                handler(req, res);
                return;
            }
        }
        throw std::runtime_error{"404"};
    }

    void plain_text(Response& res, std::string body) {
        res.headers["Content-Type"] = "text/plain";
        res.status = Status::OK;
        res.body = std::move(body);
    }
}
