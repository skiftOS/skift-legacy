#pragma once

// Based on https://github.com/managarm/libasync
// Copyright 2016-2020 libasync Contributors

#include "list.h"
#include "rc.h"
#include "res.h"
#include "vec.h"

namespace Karm::Async {

// MARK: Concepts --------------------------------------------------------------

struct Inline {};

static constexpr Inline INLINE;

struct Later {};

static constexpr Later LATER;

struct InlineOrLater {
    constexpr InlineOrLater(Inline) {}

    constexpr InlineOrLater(Later) {}
};

template <typename T>
concept Operation = requires(T t) {
    // return true if the operation is done immediately or false if it is pending
    { t.start() } -> Meta::Same<bool>;
};

template <typename R, typename T>
concept Receiver = requires(R r, T t) {
    { r.recv(INLINE, t) };
    { r.recv(LATER, t) };
};

template <typename T>
struct Null {
    void recv(auto, T) {}
};

template <typename S>
concept Sender = requires(S s) {
    { s.connect(Null<typename S::Inner>{}) } -> Operation;
};

template <typename T>
struct One {
    using Inner = T;
    T _t;

    template <Receiver<T> R>
    auto connect(R r) {
        struct Operation {
            T _t;
            R _r;

            bool start() {
                _r.recv(Async::INLINE, _t);
                return true;
            }
        };

        return Operation{_t, r};
    }
};

template <Sender S, Receiver<typename S::Inner> R>
using OperationOf = decltype(Meta::declval<S>().connect(Meta::declval<R>()));

// MARK: Basics ----------------------------------------------------------------

struct Resumable {
    virtual void resume() = 0;
    virtual ~Resumable() = default;
};

template <typename T>
struct Continuation : public Resumable {
    Opt<T> value = NONE;
};

template <Sender S>
struct [[nodiscard]] Awaiter :
    public Continuation<typename S::Inner>,
    Meta::Pinned,
    Meta::NoCopy {

    using Continuation<typename S::Inner>::value;

    struct _Receiver {
        Continuation<typename S::Inner> &_c;

        void recv(Inline, typename S::Inner t) {
            _c.value = std::move(t);
        }

        void recv(Later, typename S::Inner t) {
            _c.value = std::move(t);
            _c.resume();
        }
    };

    OperationOf<S, _Receiver> _op;
    std::coroutine_handle<> _coro = nullptr;

    Awaiter(S s)
        : _op{s.connect(_Receiver{*this})} {}

    bool await_ready() const {
        return false;
    }

    bool await_suspend(std::coroutine_handle<> coro) {
        _coro = coro;
        return not _op.start();
    }

    typename S::Inner await_resume() {
        return value.take();
    }

    void resume() override {
        _coro.resume();
    }
};

template <Sender S>
Awaiter<S> operator co_await(S s) {
    return Awaiter<S>{std::move(s)};
}

template <Sender S>
static inline typename S::Inner run(S s, auto wait) {
    Opt<typename S::Inner> ret;

    struct Receiver {
        Opt<typename S::Inner> &_ret;

        void recv(InlineOrLater, typename S::Inner r) { _ret = r; }
    };

    auto op = s.connect(Receiver{ret});
    if (op.start())
        return ret.take();
    while (not ret.has())
        wait();
    return ret.take();
}

template <Sender S>
static inline typename S::Inner run(S s) {
    Opt<typename S::Inner> ret;

    struct Receiver {
        Opt<typename S::Inner> &_ret;

        void recv(InlineOrLater, typename S::Inner r) { _ret = r; }
    };

    auto op = s.connect(Receiver{ret});
    if (not op.start()) [[unlikely]]
        panic("run() called on pending operation without a wait function");
    return ret.take();
}

template <Sender S, typename Cb>
static inline void detach(S s, Cb cb) {
    struct _Holder {
        virtual void finalize(typename S::Inner) = 0;
        virtual ~_Holder() = default;
    };

    struct Receiver {
        _Holder *_h;

        void recv(InlineOrLater, typename S::Inner r) {
            _h->finalize(std::move(r));
            // After this point, `_d` is invalid and `this` is dangling.
        }
    };

    struct Holder : public _Holder {
        using Op = OperationOf<S, Receiver>;
        Op _op;
        Cb _cb;

        Holder(S s, Cb cb)
            : _op{s.connect(Receiver{this})}, _cb{std::move(cb)} {}

        void finalize(typename S::Inner r) override {
            auto cb = std::move(_cb);
            delete this;
            cb(r);
        }

        void start() {
            _op.start();
        }
    };

    (new Holder(std::move(s), std::move(cb)))->start();
}

template <Sender S>
static inline void detach(S s) {
    detach(std::move(s), [](auto) {
    });
}

// MARK: Cancelation -----------------------------------------------------------

struct Cancelation : Meta::Pinned {
    struct Token {
        Cancelation *_c = nullptr;

        Token() = default;

        Token(Cancelation &c) : _c{&c} {}

        bool canceled() const {
            return _c and _c->canceled();
        }

        Res<> errorIfCanceled() const {
            if (canceled())
                return Error::interrupted("operation canceled");
            return Ok();
        }
    };

    bool _canceled = false;

    void cancel() {
        _canceled = true;
    }

    void reset() {
        _canceled = false;
    }

    bool canceled() const {
        return _canceled;
    }

    Token token() {
        return Token{*this};
    }
};

using Ct = Cancelation::Token;

// MARK: Promise ---------------------------------------------------------------

template <typename T>
struct State {
    struct Listener : public Resumable {
        LlItem<Listener> item;
    };

    Opt<T> _value;
    Ll<Listener> _queue;

    void set(T value) {
        if (_value.has()) [[unlikely]]
            panic("promise already resolved");
        _value = std::move(value);
        wake();
    }

    void wake() {
        while (_queue.head()) {
            auto listener = _queue.detach(_queue.head());
            listener->resume();
        }
    }

    bool has() const {
        return _value.has();
    }

    T &unwrap() {
        if (not _value.has()) [[unlikely]]
            panic("promise not resolved");
        return _value.unwrap();
    }

    void attach(Listener &listener) {
        _queue.append(&listener, _queue.tail());
    }

    void detach(Listener &listener) {
        _queue.detach(&listener);
    }
};

template <typename T>
struct _Future {
    using Inner = T;

    Strong<State<T>> _state;

    template <Receiver<T> R>
    struct _Operation :
        public State<T>::Listener {

        Strong<State<T>> _state;
        R _r;

        _Operation(Strong<State<T>> state, R r)
            : _state{std::move(state)}, _r{std::move(r)} {}

        ~_Operation() {
            _state->detach(*this);
        }

        void resume() {
            _r.recv(Async::LATER, _state->unwrap());
        }

        bool start() {
            if (not _state->has()) {
                _state->attach(*this);
                return false;
            }
            _r.recv(Async::INLINE, _state->unwrap());
            return true;
        }
    };

    template <Receiver<T> R>
    auto connect(R r) {
        return _Operation<R>{_state, std::move(r)};
    }
};

template <typename V = None, typename E = Error>
using Future = _Future<Res<V, E>>;

template <typename T>
struct _Promise : Meta::NoCopy {
    Opt<Strong<State<T>>> _state;

    _Promise() : _state{makeStrong<State<T>>()} {}

    void resolve(T value) {
        if (not _state.has()) [[unlikely]]
            panic("promise already resolved");
        _state.take()->set(std::move(value));
    }

    _Future<T> future() {
        return _Future<T>{_state.unwrap()};
    }
};

template <typename V = None, typename E = Error>
using Promise = _Promise<Res<V, E>>;

// MARK: Queue -----------------------------------------------------------------

template <typename T>
struct Queue {
    struct Listener : public Resumable {
        LlItem<Listener> item;
    };

    Vec<T> _buffer;
    Ll<Listener> _listeners;

    void equeue(T value) {
        if (_listeners.empty()) {
            _buffer.pushBack(std::move(value));
            return;
        }
 
        //TODO
    }

    _Future<T> dequeueAsync();

    Opt<T> dequeue() {
        if (_buffer.empty())
            return NONE;
        return _buffer.popFont();
    }

    bool empty() const {
        return _buffer.empty();
    }
};

// MARK: Task ------------------------------------------------------------------

enum struct Cfp {
    INDETERMINATE,
    PAST_START,
    PAST_SUSPEND
};

template <typename T>
struct _Task {
    using Inner = T;

    struct promise_type {
        Continuation<T> *_resume = nullptr;
        Cfp _cfp = Cfp::INDETERMINATE;

        _Task get_return_object() {
            return _Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        void unhandled_exception() {
            panic("unhandled exception in coroutine");
        }

        void return_value(T value) {
            _resume->value = std::move(value);
        }

        template <typename U>
        void return_value(U &&value) {
            _resume->value.emplace(std::forward<U>(value));
        }

        std::suspend_always initial_suspend() {
            return {};
        }

        auto final_suspend() noexcept {
            struct Awaiter {
                promise_type *_promise;

                bool await_ready() noexcept {
                    return false;
                }

                void await_suspend(std::coroutine_handle<void>) noexcept {
                    auto cfp = std::exchange(_promise->_cfp, Cfp::PAST_SUSPEND);
                    if (cfp == Cfp::PAST_START)
                        _promise->_resume->resume();
                }

                void await_resume() noexcept {
                    panic("coroutine resumed from final suspension point");
                }
            };

            return Awaiter{this};
        }
    };

    std::coroutine_handle<promise_type> _coro;

    _Task(std::coroutine_handle<promise_type> coro = nullptr)
        : _coro{coro} {}

    _Task(_Task const &other) = delete;

    _Task(_Task &&other)
        : _coro(std::exchange(other._coro, nullptr)) {}

    _Task &operator=(_Task const &other) = delete;

    _Task &operator=(_Task &&other) {
        std::swap(_coro, other._coro);
        return *this;
    }

    ~_Task() {
        if (_coro)
            _coro.destroy();
    }

    template <typename R>
    auto connect(R r) {
        struct Operation : public Continuation<T> {
            using Continuation<T>::value;

            _Task _t;
            R _r;

            Operation(_Task t, R r)
                : _t{std::move(t)}, _r{std::move(r)} {}

            bool start() {
                auto coro = _t._coro;
                promise_type &prom = coro.promise();
                prom._resume = this;

                coro.resume();
                auto cfp = std::exchange(prom._cfp, Cfp::PAST_START);
                if (cfp != Cfp::PAST_SUSPEND)
                    return false;

                _r.recv(Async::INLINE, value.take());
                return true;
            }

            void resume() override {
                _r.recv(Async::LATER, value.take());
            }
        };

        return Operation{std::move(*this), std::move(r)};
    }
};

template <typename V = None, typename E = Error>
using Task = _Task<Res<V, E>>;

template <Sender S>
_Task<typename S::Inner> makeTask(S s) {
    co_return co_await std::move(s);
}

} // namespace Karm::Async
