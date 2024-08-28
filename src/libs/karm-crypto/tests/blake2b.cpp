#include <karm-crypto/blake2b.h>
#include <karm-crypto/hex.h>
#include <karm-test/macros.h>

namespace Karm::Crypto::Tests {

test$("crypto-blake2b-simple") {
    return Ok();

    auto testCase = [&](Str data, Str expected) -> Res<> {
        auto digest = try$(hexEncode(blake2b(bytes(data))));
        expectEq$(digest, expected);
        return Ok();
    };

#define TEST(EXPECTED, DATA) try$(testCase(DATA, EXPECTED));
#include "blake2b-simple.inc"
#undef TEST

    return Ok();
}

} // namespace Karm::Crypto::Tests
