#pragma once

#include "server.h"

namespace Karm::Net {

struct Route {
    using Params = Map<String, String>;

    virtual ~Route() = default;

    virtual bool accept(Scope const &scope) const = 0;

    virtual Async::Task<> handleAsync(Scope const &scope, Strong<Recv> recv, Strong<Send> send) = 0;
};

struct HttpRoute : public Route {};

Strong<Route> get(Str path, Asgi handler);

Strong<Route> post(Str path, Asgi handler);

Strong<Route> put(Str path, Asgi handler);

Strong<Route> del(Str path, Asgi handler);

Strong<Route> staticFile(Str path, Mime::Url url);

struct WsRoute : public Route {};

Strong<Route> ws(Str path, Asgi handler);

Asgi router(Vec<Strong<Route>> &&routes);

template <typename... Routes>
static inline Asgi router(Routes &&...routes) {
    return router({std::forward<Routes>(routes)...});
}

} // namespace Karm::Net
