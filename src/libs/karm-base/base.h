#pragma once

#include <karm-meta/traits.h>

#include "_prelude.h"

namespace Karm {

// MARK: Macros ----------------------------------------------------------------

// Banned functions and features

#define goto _Pragma("GCC error \"goto is not allowed\"") goto
#define throw _Pragma("GCC error \"throw is not allowed\"") throw
#define try _Pragma("GCC error \"try is not allowed\"") try
#define catch _Pragma("GCC error \"catch is not allowed\"") catch
#define throw_unless _Pragma("GCC error \"throw_unless is not allowed\"") throw_unless
#define dynamic_cast _Pragma("GCC error \"dynamic_cast is not allowed\"") dynamic_cast

// Safety

// https://clang.llvm.org/docs/AttributeReference.html#lifetimebound
#define lifetimebound [[clang::lifetimebound]]

// Utilities

#define arrayLen$(ARR) (sizeof(ARR) / sizeof(ARR[0]))

#define __concat$(LHS, RHS) LHS##RHS
#define concat$(LHS, RHS) __concat$(LHS, RHS)

#define __stringify$(SYM) #SYM
#define stringify$(SYM) __stringify$(SYM)

#define var$(NAME) concat$(__m_##NAME##_, __LINE__)

#define __count$(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
#define count$(__args...) __count$(0, ##__args, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Eval

#define __eval0$(...) __VA_ARGS__
#define __eval1$(...) __eval0$(__eval0$(__eval0$(__VA_ARGS__)))
#define __eval2$(...) __eval1$(__eval1$(__eval1$(__VA_ARGS__)))
#define __eval3$(...) __eval2$(__eval2$(__eval2$(__VA_ARGS__)))
#define __eval4$(...) __eval3$(__eval3$(__eval3$(__VA_ARGS__)))
#define eval$(...) __eval4$(__eval4$(__eval4$(__VA_ARGS__)))

// Map

#define __MAP_END__(...)
#define __MAP_OUT__
#define __MAP_COMMA__ ,

#define __mapGetEnd2() 0, __MAP_END__
#define __mapGetEnd1(...) __mapGetEnd2
#define __mapGetEnd(...) __mapGetEnd1

#define __mapNext0(test, next, ...) next __MAP_OUT__
#define __mapNext1(test, next) __mapNext0(test, next, 0)
#define __mapNext(test, next) __mapNext1(__mapGetEnd test, next)

#define __map0(f, x, peek, ...) f(x) __mapNext(peek, __map1)(f, peek, __VA_ARGS__)
#define __map1(f, x, peek, ...) f(x) __mapNext(peek, __map0)(f, peek, __VA_ARGS__)

#define __mapListNext1(test, next) __mapNext0(test, __MAP_COMMA__ next, 0)
#define __mapListNext(test, next) __mapListNext1(__mapGetEnd test, next)

#define __mapList0(f, x, peek, ...) f(x) __mapListNext(peek, __mapList1)(f, peek, __VA_ARGS__)
#define __mapList1(f, x, peek, ...) f(x) __mapListNext(peek, __mapList0)(f, peek, __VA_ARGS__)

// Applies the function macro `f` to each of the remaining parameters.
#define map$(f, ...) eval$(__map1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies the function macro `f` to each of the remaining parameters and
// inserts commas between the results.
#define mapList$(f, ...) eval$(__mapList1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Constructs

#define always_inline [[gnu::always_inline]]

// MARK: Unsigned --------------------------------------------------------------

using usize = __SIZE_TYPE__;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
#ifdef __SIZEOF_INT128__
using u128 = __uint128_t;
#endif

template <typename T>
    requires(sizeof(T) <= 8)
always_inline constexpr T bswap(T value) {
    if (sizeof(T) == 8)
        return __builtin_bswap64(value);
    if (sizeof(T) == 4)
        return __builtin_bswap32(value);
    if (sizeof(T) == 2)
        return __builtin_bswap16(value);
    if (sizeof(T) == 1)
        return value;
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
template <typename T>
always_inline constexpr T toLe(T value) {
    return value;
}
#else
template <typename T>
always_inline constexpr T toLe(T value) {
    return bswap(value);
}
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
template <typename T>
always_inline constexpr T toBe(T value) {
    return bswap(value);
}
#else
template <typename T>
always_inline constexpr T toBe(T value) {
    return value;
}
#endif

template <typename T>
always_inline constexpr usize popcount(T value) {
    usize count = 0;
    for (usize i = 0; i < sizeof(T) * 8; i++)
        if (value & (1uz << i))
            count++;
    return count;
}

template <typename T>
always_inline constexpr T rol(T x, usize n) {
    return (x << n) | (x >> (sizeof(x) * 8 - n));
}

template <typename T>
always_inline constexpr T ror(T x, usize n) {
    return (x >> n) | (x << (sizeof(x) * 8 - n));
}

template <typename T>
always_inline constexpr T rotl(T x, usize n) {
    return (x << n) | (x >> (sizeof(x) * 8 - n));
}

template <typename T>
always_inline constexpr T rotr(T x, usize n) {
    return (x >> n) | (x << (sizeof(x) * 8 - n));
}

// MARK: Signed -----------------------------------------------------------

using isize = __PTRDIFF_TYPE__;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

#ifdef __SIZEOF_INT128__
using i128 = __int128_t;
#endif

// MARK: Floating point --------------------------------------------------------

using f16 = __fp16;
using f32 = float;
using f64 = double;
using f128 = long double;

// MARK: Boolean type ----------------------------------------------------------

struct Bool {
    bool _val;

    Bool() = delete;

    constexpr Bool(Meta::Boolean auto value)
        : _val(value) {}

    constexpr Bool &operator=(Meta::Boolean auto value) {
        _val = value;
        return *this;
    }

    explicit constexpr operator bool() const {
        return _val;
    }

    constexpr Bool operator!() const {
        return Bool(not _val);
    }
};

constexpr bool operator==(Bool a, Bool b) {
    return static_cast<bool>(a) == static_cast<bool>(b);
}

constexpr bool operator==(Bool a, Meta::Boolean auto b) {
    return static_cast<bool>(a) == static_cast<bool>(b);
}

constexpr bool operator==(Meta::Boolean auto a, Bool &b) {
    return static_cast<bool>(a) == static_cast<bool>(b);
}

constexpr bool operator!=(Bool a, Bool b) {
    return static_cast<bool>(a) != static_cast<bool>(b);
}

constexpr bool operator!=(Bool a, Meta::Boolean auto b) {
    return static_cast<bool>(a) != static_cast<bool>(b);
}

constexpr bool operator!=(Meta::Boolean auto a, Bool b) {
    return static_cast<bool>(a) != static_cast<bool>(b);
}

// MARK: Tags ------------------------------------------------------------------

/* The object should take the ownership of the memory */

struct Move {};

constexpr inline auto MOVE = Move{};

/* The object should make a copy of the memory */

struct Copy {};

constexpr inline auto COPY = Copy{};

/* The object should wrap the memory without doing a copy
   nor taking the ownership */

struct Wrap {};

constexpr inline auto WRAP = Wrap{};

/* The object should be empty initialized */

struct None {
    constexpr None() {}

    explicit operator bool() const { return false; }

    bool operator==(None const &) const = default;

    auto operator<=>(None const &) const = default;

    template <typename T>
    operator T *() const {
        return nullptr;
    }
};

constexpr inline auto NONE = None{};

template <typename T>
bool operator==(None, T *ptr) {
    return ptr == nullptr;
}

// MARK: Misc ------------------------------------------------------------------

/// A linker symbol.
using ExternSym = uint8_t[];

// MARK: Utilities -------------------------------------------------------------

template <typename T, typename U>
always_inline static inline T unionCast(U value)
    requires(sizeof(T) == sizeof(U))
{
    union X {
        U u;
        T t;
    };

    return X{.u = value}.t;
}

} // namespace Karm
