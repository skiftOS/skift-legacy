#pragma once

#include "clamp.h"
#include "opt.h"

namespace Karm {

template <typename Next>
struct Iter {
    Next next;
    using Item = decltype(next());
    using Value = Meta::RemoveConstVolatileRef<decltype(*next())>;

    constexpr Iter(Next next) : next(next) {}

    constexpr auto map(auto f) {
        auto n = [=, *this] mutable -> Opt<decltype(f(*Meta::declval<Item>()))> {
            auto v = next();

            if (not v) {
                return NONE;
            }

            return f(*v);
        };

        return Iter<decltype(n)>{n};
    }

    constexpr auto mapi(auto f) {
        auto n = [=, *this, index = 0uz] mutable -> Opt<decltype(f(*Meta::declval<Item>(), 1uz))> {
            auto v = next();
            index++;

            if (not v) {
                return NONE;
            }

            return f(*v, index - 1);
        };

        return Iter<decltype(n)>{n};
    }

    constexpr auto filter(auto f) {
        auto n = [=, *this] mutable -> Item {
            auto v = next();
            if (not v) {
                return NONE;
            }

            while (not f(*v)) {
                v = next();
                if (not v) {
                    return NONE;
                }
            }

            return v;
        };

        return Iter<decltype(n)>{n};
    }

    constexpr auto reduce(auto init, auto f) {
        auto result = init;
        forEach([&](auto v) {
            result = f(result, v);
        });
        return result;
    }

    constexpr auto forEach(auto f) {
        for (auto item = next(); item; item = next()) {
            f(*item);
        }
    }

    constexpr auto find(auto f) -> Item {
        for (auto item = next(); item; item = next()) {
            if (f(*item)) {
                return item;
            }
        }

        return NONE;
    }

    constexpr usize len() {
        usize result = 0;
        forEach([&](auto) {
            result++;
        });
        return result;
    }

    constexpr auto sum() {
        return reduce(Value{}, [](auto &a, auto &b) {
            return a + b;
        });
    }

    constexpr bool any() {
        return next() != NONE;
    }

    constexpr auto first() {
        return next();
    }

    constexpr auto first(auto f) {
        for (auto item = next(); item; item = next()) {
            if (f(*item)) {
                return item;
            }
        }

        return NONE;
    }

    constexpr auto last() {
        Item result = NONE;
        for (auto item = next(); item; item = next()) {
            result = item;
        }
        return result;
    }

    constexpr auto last(auto f) {
        Item result = NONE;
        for (auto item = next(); item; item = next()) {
            if (f(*item)) {
                result = item;
            }
        }
        return result;
    }

    constexpr auto skip(usize n) {
        for (usize i = 0; i < n; i++) {
            next();
        }
        return *this;
    }

    constexpr auto cycle(usize n) {
        return Iter{[start = *this, curr = *this, i = 0, n] mutable {
            auto v = curr.next();

            if (not v and i < n) {
                curr = start;
                i++;
                v = curr.next();
            }

            return v;
        }};
    }

    constexpr auto take(usize n) {
        return Iter{[=, *this] mutable {
            if (n == 0) {
                return NONE;
            }

            auto v = next();
            n--;
            return v;
        }};
    }

    constexpr auto prepend(auto v) {
        return Iter{
            [=, *this, first = true] mutable -> Item {
                if (first)
                    return Item{v};

                return next();
            }
        };
    }

    constexpr auto append(auto v) {
        return Iter{
            [=, *this, last = true] mutable -> Item {
                auto result = next();

                if (not result and last) {
                    last = false;
                    return Item{v};
                }

                return result;
            }
        };
    }

    constexpr auto at(usize n) {
        return skip(n).first();
    }

    constexpr bool any(auto f) {
        return find(f) != NONE;
    }

    constexpr bool all(auto f) {
        for (auto item = next(); item; item = next()) {
            if (not f(*item)) {
                return false;
            }
        }

        return true;
    }

    constexpr auto min() {
        Item result = NONE;

        for (auto item = next(); item; item = next()) {
            if (not result or (*item < *result)) {
                result = item;
            }
        }

        return result;
    }

    constexpr auto max() {
        Item result = NONE;

        for (auto item = next(); item; item = next()) {
            if (not result or (*item > *result)) {
                result = item;
            }
        }

        return result;
    }

    constexpr auto avg() -> Item {
        Item result = NONE;
        usize count = 0;

        for (auto item = next(); item; item = next()) {
            if (not result) {
                result = item;
            } else {
                result = *result + *item;
            }
            count++;
        }

        if (count == 0) {
            return NONE;
        }

        return *result / count;
    }

    struct It {
        Item curr;
        Iter iter;

        constexpr auto &operator*() {
            return *curr;
        }

        constexpr auto operator++() {
            curr = iter.next();
            return *this;
        }

        constexpr bool operator!=(None) {
            return curr != NONE;
        }
    };

    constexpr It begin() {
        return It{next(), *this};
    }

    constexpr None end() {
        return NONE;
    }

    constexpr auto collect() {
        forEach([&](auto const &...) {
        });
    }

    template <typename C>
    constexpr auto collect(C &c) {
        forEach([&](auto v) {
            c.pushBack(v);
        });
    }

    template <typename C>
    constexpr auto collect(C &c, auto f) {
        forEach([&](auto v) {
            c.pushBack(f(v));
        });
    }

    template <typename C>
    constexpr auto collect() {
        C c;
        collect(c);
        return c;
    }

    template <typename C>
    constexpr auto collect(auto f) {
        C c;
        collect(c, f);
        return c;
    }
};

template <typename T>
constexpr auto single(T value) {
    return Iter<None>{[value, end = false] mutable {
        if (end) {
            return NONE;
        }

        end = true;
        return value;
    }};
}

template <typename T>
constexpr auto repeat(T value, usize count) {
    return Iter{[value, count] mutable -> Opt<T> {
        if (count == 0) {
            return NONE;
        }

        count--;
        return value;
    }};
}

template <typename T>
constexpr auto range(T end) {
    return Iter{[value = static_cast<T>(0), end] mutable -> Opt<T> {
        if (value >= end) {
            return NONE;
        }

        return value++;
    }};
}

template <typename T>
constexpr auto range(T start, T end) {
    return Iter{[value = start, start, end] mutable -> Opt<T> {
        if (value >= end) {
            return NONE;
        }

        auto result = value;
        if (start < end) {
            value++;
        } else {
            value++;
        }
        return result;
    }};
}

template <typename T>
constexpr auto range(T start, T end, T step) {
    return Iter{[value = start, start, end, step] mutable -> Opt<T> {
        if (value >= end) {
            return NONE;
        }

        auto result = value;
        if (start < end) {
            value += step;
        } else {
            value -= step;
        }
        return result;
    }};
}

template <typename T>
concept AtLen = requires(T t) {
    t.len();
    t.at(0);
};

constexpr auto iter(AtLen auto &o) {
    return range(o.len())
        .map([&](auto i) mutable {
            return o.at(i);
        });
}

} // namespace Karm
