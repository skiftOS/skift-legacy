#pragma once

#include <karm-base/async.h>
#include <karm-base/func.h>
#include <karm-json/parse.h>
#include <karm-mime/url.h>

namespace Karm::Net {

struct Scope {
    enum struct _Type {
        WEBSOCKET,
        HTTP,
    };

    using enum _Type;

    struct _Data {
        _Type type;
        Mime::Url url;
        Map<String, String> headers;
        Map<String, String> params;
    };

    Strong<_Data> _data;
};

struct Recv {
    struct Impl {
        virtual ~Impl() = default;
    };

    Strong<Impl> _impl;

    Async::Task<Json::Value> jsonAsync() const;
};

struct Send {
    struct Impl {
        virtual ~Impl() = default;
    };

    Strong<Impl> _impl;
};

using Asgi = SharedFunc<Async::Task<>(Scope scope, Recv recv, Send send)>;

struct ServProps {
};

Async::Task<> servAsync(Asgi app, ServProps props = {});

} // namespace Karm::Net
