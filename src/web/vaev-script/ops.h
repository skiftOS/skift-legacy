#pragma once

#include "agent.h"
#include "completion.h"
#include "value.h"

namespace Vaev::Script {

// MARK: Abstract Operations ===================================================
// https://tc39.es/ecma262/#sec-abstract-operations

// MARK: Type Conversion -------------------------------------------------------

// MARK: Testing and Comparison Operations -------------------------------------

// https://tc39.es/ecma262/#sec-iscallable
bool isCallable(Value v);

// MARK: Operations on Objects -------------------------------------------------

// https://tc39.es/ecma262/#sec-call
Except<Value> call(Agent &agent, Value f, Value v, Slice<Value> args = {});

// MARK: Operations on Iterator Objects ----------------------------------------

// MARK: Runtime Semantics =====================================================

enum struct ExceptionType {
    ERROR,
    TYPE_ERROR,
    RANGE_ERROR,
    REFERENCE_ERROR,
    SYNTAX_ERROR,
    URI_ERROR,

    _LEN,
};

Gc::Ref<Object> createException(Agent &agent, ExceptionType type);

// https://tc39.es/ecma262/#sec-throw-an-exception
Completion throwException(Value exception);

} // namespace Vaev::Script
