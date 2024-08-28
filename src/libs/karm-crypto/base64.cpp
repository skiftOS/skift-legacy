#include <karm-io/impls.h>

#include "base64.h"

namespace Karm::Crypto {

static constexpr Array<Rune, 64> BASE64_TABLE = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static constexpr u8 BASE64_PAD = '=';

Base64Encoder::Base64Encoder(Io::TextWriter &out)
    : _out(out) {}

Res<> Base64Encoder::write(u8 byte) {
    _buf[_len++] = byte;

    if (_len == 3) {
        try$(_out.writeRune(BASE64_TABLE[_buf[0] >> 2]));
        try$(_out.writeRune(BASE64_TABLE[((_buf[0] & 0x03) << 4) | (_buf[1] >> 4)]));
        try$(_out.writeRune(BASE64_TABLE[((_buf[1] & 0x0F) << 2) | (_buf[2] >> 6)]));
        try$(_out.writeRune(BASE64_TABLE[_buf[2] & 0x3F]));

        _len = 0;
    }

    return Ok();
}

Res<> Base64Encoder::flush() {
    if (_len == 1) {
        try$(_out.writeRune(BASE64_TABLE[_buf[0] >> 2]));
        try$(_out.writeRune(BASE64_TABLE[(_buf[0] & 0x03) << 4]));
        try$(_out.writeRune(BASE64_PAD));
        try$(_out.writeRune(BASE64_PAD));
    } else if (_len == 2) {
        try$(_out.writeRune(BASE64_TABLE[_buf[0] >> 2]));
        try$(_out.writeRune(BASE64_TABLE[((_buf[0] & 0x03) << 4) | (_buf[1] >> 4)]));
        try$(_out.writeRune(BASE64_TABLE[(_buf[1] & 0x0F) << 2]));
        try$(_out.writeRune(BASE64_PAD));
    }

    return Ok();
}

Res<> Base64Encoder::write(Bytes bytes) {
    for (usize i = 0; i < bytes.len(); i++) {
        try$(write(bytes[i]));
    }

    return Ok();
}

Res<> Base64Encoder::write(Io::Reader &in) {
    Array<u8, 4096> buf;
    while (usize read = try$(in.read(buf)))
        try$(write(sub(buf, 0, read)));
    try$(flush());

    return Ok();
}

Res<String> base64Encode(Bytes bytes) {
    Io::StringWriter writer;
    Base64Encoder encoder{writer};
    try$(encoder.write(bytes));
    try$(encoder.flush());
    return Ok(writer.str());
}

} // namespace Karm::Crypto
