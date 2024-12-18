#include "ops.h"

#include "object.h"

namespace Vaev::Script {

// MARK: Abstract Operations ---------------------------------------------------
// https://tc39.es/ecma262/#sec-abstract-operations

// MARK: Type Conversion -------------------------------------------------------

// MARK: Testing and Comparison Operations -------------------------------------

// https://tc39.es/ecma262/#sec-iscallable
bool isCallable(Value v) {
    // 1. If argument is not an Object, return false.
    if (not v.isObject())
        return false;

    // 2. If argument has a [[Call]] internal method, return true.

    // 3. Return false.
    return false;
}

// MARK: Operations on Objects -------------------------------------------------

// https://tc39.es/ecma262/#sec-call
Except<Value> call(Agent &agent, Value f, Value v, Slice<Value> args) {
    // 1. If argumentsList is not present, set argumentsList to a new empty List.
    // 2. If IsCallable(F) is false, throw a TypeError exception.
    if (not isCallable(f))
        return throwException(createException(agent, ExceptionType::TYPE_ERROR));

    // 3. Return ? F.[[Call]](V, argumentsList)
    return f.asObject()->call(v.asObject(), args);
}

// MARK: Operations on Iterator Objects ----------------------------------------

// MARK: Runtime Semantics -----------------------------------------------------

// https://tc39.es/ecma262/#sec-throw-an-exception

Gc::Ref<Object> createException(Agent &agent, ExceptionType) {
    // FIXME: Implement this properly.
    auto exception = Object::create(agent);
    exception->set(
        PropertyKey::from(u"name"_s16),
        String{u"Exception"_s16},
        exception
    );
    return exception;
}

Completion throwException(Value exception) {
    // 1. Return ThrowCompletion(a newly created TypeError object).
    return Completion::throw_(exception);
}

} // namespace Vaev::Script
