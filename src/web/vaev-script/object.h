#pragma once

#include <karm-base/vec.h>

#include "agent.h"
#include "ops.h"
#include "value.h"

namespace Vaev::Script {

// https://tc39.es/ecma262/#property-key

struct PropertyKey {
    using _Store = Union<String, Symbol, u64>;

    _Store store;

    static PropertyKey from(String str) {
        return {str};
    }

    static PropertyKey from(Symbol sym) {
        return {sym};
    }

    static PropertyKey from(u64 num) {
        return {num};
    }
};

// https://tc39.es/ecma262/#sec-property-attributes
struct PropertyDescriptor {
    Value value = UNDEFINED;
    Boolean writable = false;
    Value get = UNDEFINED;
    Value set = UNDEFINED;
    Boolean enumerable = false;
    Boolean configurable = false;

    // https://tc39.es/ecma262/#sec-isaccessordescriptor
    Boolean isAccessorDescriptor() const {
        // 1. If Desc is undefined, return false.

        // 2. If Desc has a [[Get]] or [[Set]] field, return true.
        if (get != UNDEFINED or set != UNDEFINED)
            return true;

        // 3. Return false.
        return false;
    }

    // https://tc39.es/ecma262/#sec-isdatadescriptor
    Boolean isDataDescriptor() const {
        // 1. If Desc is undefined, return false.

        // 2. If Desc has a [[Value]] field, return true.
        if (value != UNDEFINED)
            return true;

        // 3. If Desc has a [[Writable]] field, return true.
        if (writable)
            return true;

        // 4. Return false.
        return false;
    }
};

// https://tc39.es/ecma262/#sec-object-type

struct _ObjectCreateArgs {
    Opt<Gc::Ref<Object>> prototype = NONE;
};

struct Object {
    Agent &agent;

    static Gc::Ref<Object> create(Agent &agent, _ObjectCreateArgs args = {});

    // https://tc39.es/ecma262/#table-essential-internal-methods
    Opt<Gc::Ref<Object>> getPrototypeOf() const;

    Boolean setPrototypeOf(Gc::Ref<Object> proto);

    Boolean isExtensible() const;

    Boolean preventExtensions();

    Union<PropertyDescriptor, Undefined> getOwnProperty(PropertyKey key) const;

    // https://tc39.es/ecma262/#sec-ordinarydefineownproperty
    Except<Boolean> ordinaryDefineOwnProperty(PropertyKey key, PropertyDescriptor desc) {
        // 1. Let current be ? O.[[GetOwnProperty]](P).
        auto maybeCurrent = getOwnProperty(key);

        // 2. Let extensible be ? IsExtensible(O).
        // 3. Return ValidateAndApplyPropertyDescriptor(O, P, extensible, Desc, current).
    }

    // https://tc39.es/ecma262/#sec-ordinary-object-internal-methods-and-internal-slots-defineownproperty-p-desc
    Except<Boolean> defineOwnProperty(PropertyKey key, PropertyDescriptor desc) {
        // 1. Return ? OrdinaryDefineOwnProperty(O, P, Desc).
        return ordinaryDefineOwnProperty(key, desc);
    }

    Boolean hasProperty(PropertyKey key) const;

    // https://tc39.es/ecma262/#sec-ordinary-object-internal-methods-and-internal-slots-get-p-receiver
    Except<Value> get(PropertyKey key, Value receiver) const {
        // 1. Return ? OrdinaryGet(O, P, Receiver).
        return ordinaryGet(key, receiver);
    }

    // https://tc39.es/ecma262/#sec-ordinaryget
    Except<Value> ordinaryGet(PropertyKey key, Value receiver) const {
        // 1. Let desc be ? O.[[GetOwnProperty]](P).
        auto maybeDesc = getOwnProperty(key);

        // 2. If desc is undefined, then
        if (maybeDesc == UNDEFINED) {
            //    a. Let parent be ? O.[[GetPrototypeOf]]().
            auto parent = getPrototypeOf();

            //    b. If parent is null, return undefined.
            if (parent == NONE)
                return Ok(UNDEFINED);

            //    c. Return ? parent.[[Get]](P, Receiver).
            return (*parent)->get(key, receiver);
        }

        auto &desc = maybeDesc.unwrap<PropertyDescriptor>();

        // 3. If IsDataDescriptor(desc) is true, return desc.[[Value]].
        if (desc.isDataDescriptor())
            return Ok(desc.value);

        // 4. Assert: IsAccessorDescriptor(desc) is true.
        if (not desc.isAccessorDescriptor())
            panic("expected accessor descriptor");

        // 5. Let getter be desc.[[Get]].
        auto getter = desc.get;

        // 6. If getter is undefined, return undefined.
        if (getter == UNDEFINED)
            return Ok(UNDEFINED);

        // 7. Return ? Call(getter, Receiver).
        return Script::call(agent, getter, receiver);
    }

    Boolean set(PropertyKey key, Value value, Value receiver);

    Boolean delete_(PropertyKey key) const;

    Vec<PropertyKey> ownPropertyKeys() const;

    // https://tc39.es/ecma262/#table-additional-essential-internal-methods-of-function-objects
    Except<Value> call(Gc::Ref<Object> thisArg, Slice<Value> args) const;

    Value construct(Slice<Value> args, Gc::Ref<Object> newTarget) const;

    void repr(Io::Emit &e) const {
        e("[object Object]");
    }
};

} // namespace Vaev::Script
