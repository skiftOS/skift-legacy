#pragma once

#include <karm-base/async.h>
#include <karm-mime/url.h>

namespace Karm::Db {

struct Db {
    virtual ~Db() = default;
};

Async::Task<Strong<Db>> connectAsync(Mime::Url);

} // namespace Karm::Db
