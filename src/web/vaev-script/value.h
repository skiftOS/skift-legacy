#pragma once

#include <karm-base/string.h>
#include <karm-base/union.h>
#include <karm-io/emit.h>

#include "gc.h"

struct Agent;

namespace Vaev::Script {

// https://tc39.es/ecma262/#sec-ecmascript-language-types-undefined-type
struct Undefined {
    bool operator==(Undefined const &) const = default;
    auto operator<=>(Undefined const &) const = default;

    void repr(Io::Emit &e) const {
        e("undefined");
    }
};

static constexpr Undefined UNDEFINED{};

// https://tc39.es/ecma262/#sec-ecmascript-language-types-null-type
struct Null {
    bool operator==(Null const &) const = default;
    auto operator<=>(Null const &) const = default;

    void repr(Io::Emit &e) const {
        e("null");
    }
};

static constexpr Null NULL_{};

// https://tc39.es/ecma262/#sec-ecmascript-language-types-boolean-type
using Boolean = bool;

// https://tc39.es/ecma262/#sec-ecmascript-language-types-string-type
using String = _String<Utf16>;

// https://tc39.es/ecma262/#sec-ecmascript-language-types-symbol-type
struct Symbol {
    void repr(Io::Emit &e) const {
        e("Symbol");
    }
};

// https://tc39.es/ecma262/#sec-ecmascript-language-types-number-type
using Number = f64;

struct Object;

struct Value {
    using _Store = Union<
        Undefined,
        Null,
        Boolean,
        String,
        Symbol,
        Number,
        Gc::Ref<Object>>;

    _Store store = UNDEFINED;

    Value() = default;

    template <Meta::Convertible<_Store> T>
    Value(T &&t) : store{std::forward<T>(t)} {}

    template <Meta::Equatable<_Store> T>
    bool operator==(T const &t) const {
        return store == t;
    }

    Boolean isObject() const {
        return store.is<Gc::Ref<Object>>();
    }

    Gc::Ref<Object> asObject() {
        return store.unwrap<Gc::Ref<Object>>();
    }

    void repr(Io::Emit &e) const {
        e("{}", store);
    }
};

} // namespace Vaev::Script
