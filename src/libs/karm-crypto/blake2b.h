#pragma once

#include <karm-base/array.h>
#include <karm-base/endian.h>

namespace Karm::Crypto {

static constexpr usize BLAKE2B_BLOCK_BYTES = 128;
static constexpr usize BLAKE2B_KEY_BYTES = 64;
static constexpr usize BLAKE2B_SALT_BYTES = 16;
static constexpr usize BLAKE2B_PERSONAL_BYTES = 16;
static constexpr usize BLAKE2B_OUT_BYTES = 64;

struct Blake2b {
    using Digest = Array<u8, BLAKE2B_OUT_BYTES>;

    struct [[gnu::packed]] Params {
        u8 digestLength;
        u8 keyLength;
        u8 fanOut;
        u8 depth;
        u32le leafLength;
        u32le nodeOffset;
        u32le xofLength;
        u8 nodeDepth;
        u8 innerLength;
        u8 reserved[14];
        u8 salt[BLAKE2B_SALT_BYTES];
        u8 personal[BLAKE2B_PERSONAL_BYTES];
    };

    Array<u64, 8> _h = {};
    Array<u64, 2> _t = {};
    Array<u64, 2> _f = {};

    u8 _buf[BLAKE2B_BLOCK_BYTES] = {};
    usize _bufLen = {};
    u8 _lastNode = {};

    void init();

    void _incrementCounter(u64 inc);

    void _compress(u64 const *block);

    void update(Bytes input);

    Digest digest();
};

Blake2b::Digest blake2b(Bytes input);

} // namespace Karm::Crypto
