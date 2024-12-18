#include <karm-sys/entry.h>
#include <vaev-script/object.h>
#include <vaev-script/realm.h>

using namespace Vaev;

Async::Task<> entryPointAsync(Sys::Context &) {
    Gc::Gc gc;

    auto agent = gc.alloc<Script::Agent>(gc);
    auto realm = gc.alloc<Script::Realm>(agent);

    (void)realm->initializeHostDefinedRealm(agent);

    auto object1 = Script::Object::create(*agent);
    object1->defineOwnProperty(Script::PropertyKey::from(u"foo"_s16), {.value = 42.});

    auto object2 = Script::Object::create(*agent, {.prototype = object1});

    auto res = object2->get(Script::PropertyKey::from(u"foo"_s16), object2);

    Sys::print("object2.foo = {}\n", res);

    co_return Ok();
}
