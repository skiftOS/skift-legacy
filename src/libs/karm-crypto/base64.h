#pragma once

#include <karm-io/traits.h>

namespace Karm::Crypto {

struct Base64Encoder {

    Io::TextWriter &_out;
    u8 _buf[3];
    u8 _len = 0;

    always_inline Base64Encoder(Io::TextWriter &out);

    always_inline Res<> write(u8 byte);

    always_inline Res<> flush();

    always_inline Res<> write(Bytes bytes);

    always_inline Res<> write(Io::Reader &in);
};

Res<String> base64Encode(Bytes bytes);

} // namespace Karm::Crypto
