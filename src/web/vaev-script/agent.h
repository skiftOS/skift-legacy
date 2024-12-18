#pragma once

#include <karm-base/res.h>

#include "gc.h"

namespace Vaev::Script {

// https://tc39.es/ecma262/#agent
struct Agent {
    Gc::Gc &gc;
};

} // namespace Vaev::Script
