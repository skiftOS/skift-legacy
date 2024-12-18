#pragma once

#include <karm-base/base.h>
#include <karm-base/panic.h>
#include <karm-io/emit.h>

namespace Karm::Gc {

template <typename T>
struct Ref {
    T *_ptr = nullptr;

    Ref() = default;

    Ref(T *ptr) : _ptr{ptr} {
        if (not _ptr)
            panic("null pointer");
    }

    T const *operator->() const {
        return _ptr;
    }

    T *operator->() {
        return _ptr;
    }

    T const &operator*() const {
        return *_ptr;
    }

    T &operator*() {
        return *_ptr;
    }

    void repr(Io::Emit &e) const {
        e("{}", *_ptr);
    }
};

struct Gc {
    template <typename T, typename... Args>
    Ref<T> alloc(Args &&...args) {
        return new T{std::forward<Args>(args)...};
    }
};

} // namespace Karm::Gc
