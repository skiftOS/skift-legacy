#pragma once

#include "agent.h"
#include "completion.h"
#include "value.h"

namespace Vaev::Script {

// https://tc39.es/ecma262/#realm
struct Realm {
    Gc::Ref<Agent> agentSignifier;
    Value globalThis = UNDEFINED;

    // https://tc39.es/ecma262/#sec-createintrinsics
    Completion createIntrinsics() {
        // 1. Set realmRec.[[Intrinsics]] to a new Record.

        // 2. Set fields of realmRec.[[Intrinsics]] with the values listed in Table 6. The field names are the names listed in column one of the table. The value of each field is a new object value fully and recursively populated with property values as defined by the specification of each object in clauses 19 through 28. All object property values are newly created object values. All values that are built-in function objects are created by performing CreateBuiltinFunction(steps, length, name, slots, realmRec, prototype) where steps is the definition of that function provided by this specification, name is the initial value of the function's "name" property, length is the initial value of the function's "length" property, slots is a list of the names, if any, of the function's specified internal slots, and prototype is the specified value of the function's [[Prototype]] internal slot. The creation of the intrinsics and their properties must be ordered to avoid any dependencies upon objects that have not yet been created.

        // 3. Perform AddRestrictedFunctionProperties(realmRec.[[Intrinsics]].[[%Function.prototype%]], realmRec).

        // 4. Return unused.
        return Completion::normal(UNDEFINED);
    }

    // https://tc39.es/ecma262/#sec-initializehostdefinedrealm
    Completion initializeHostDefinedRealm(Gc::Ref<Agent>) {
        // 1. Let realm be a new Realm Record.

        // 2. Perform CreateIntrinsics(realm).

        // 3. Set realm.[[AgentSignifier]] to AgentSignifier().

        // 4. Set realm.[[GlobalObject]] to undefined.

        // 5. Set realm.[[GlobalEnv]] to undefined.

        // 6. Set realm.[[TemplateMap]] to a new empty List.

        // 7. Let newContext be a new execution context.

        // 8. Set the Function of newContext to null.

        // 9. Set the Realm of newContext to realm.

        // 10. Set the ScriptOrModule of newContext to null.

        // 11. Push newContext onto the execution context stack; newContext is now the running execution context.

        // 12. If the host requires use of an exotic object to serve as realm's global object, then
        //     a. Let global be such an object created in a host-defined manner.

        // 13. Else,
        //     a. Let global be OrdinaryObjectCreate(realm.[[Intrinsics]].[[%Object.prototype%]]).

        // 14. If the host requires that the this binding in realm's global scope return an object other than the global object, then
        //     a. Let thisValue be such an object created in a host-defined manner.

        // 15. Else,
        //     a. Let thisValue be global.

        // 16. Set realm.[[GlobalObject]] to global.

        // 17. Set realm.[[GlobalEnv]] to NewGlobalEnvironment(global, thisValue).

        // 18. Perform ? SetDefaultGlobalBindings(realm).

        // 19. Create any host-defined global object properties on global.

        // 20. Return unused.
        return Completion::normal(UNDEFINED);
    }
};

} // namespace Vaev::Script
