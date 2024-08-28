#include "blake2b.h"

namespace Karm::Crypto {

static u64 const BLAKE2B_IV[8] = {
    0x6a09e667f3bcc908,
    0xbb67ae8584caa73b,
    0x3c6ef372fe94f82b,
    0xa54ff53a5f1d36f1,
    0x510e527fade682d1,
    0x9b05688c2b3e6c1f,
    0x1f83d9abfb41bd6b,
    0x5be0cd19137e2179,
};

static u8 const BLAKE2B_SIGMA[12][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
};

void Blake2b::init() {
    struct Params param = {};

    param.digestLength = BLAKE2B_OUT_BYTES;
    param.fanOut = 1;
    param.depth = 1;

    for (int i = 0; i < 8; i++) {
        _h[i] = BLAKE2B_IV[i];
    }

    for (int i = 0; i < 8; i++) {
        _h[i] ^= *(u64 *)((u8 *)&param + sizeof(_h[i]) * i);
    }
}

void Blake2b::_incrementCounter(u64 inc) {
    _t[0] += inc;
    _t[1] += _t[0] < inc;
}

void Blake2b::_compress(u64 const *block) {
    Array<u64, 16> m;
    Array<u64, 16> v;

    for (int i = 0; i < 16; i++) {
        m[i] = block[i];
    }

    for (int i = 0; i < 8; i++) {
        v[i] = _h[i];
    }

    v[8] = BLAKE2B_IV[0];
    v[9] = BLAKE2B_IV[1];
    v[10] = BLAKE2B_IV[2];
    v[11] = BLAKE2B_IV[3];
    v[12] = BLAKE2B_IV[4] ^ _t[0];
    v[13] = BLAKE2B_IV[5] ^ _t[1];
    v[14] = BLAKE2B_IV[6] ^ _f[0];
    v[15] = BLAKE2B_IV[7] ^ _f[1];

    auto G = [&](usize r, usize i, u64 &a, u64 &b, u64 &c, u64 &d) {
        a = a + b + m[BLAKE2B_SIGMA[r][2 * i + 0]];
        d = rotr<u64>(d ^ a, 32);
        c = c + d;
        b = rotr<u64>(b ^ c, 24);
        a = a + b + m[BLAKE2B_SIGMA[r][2 * i + 1]];
        d = rotr<u64>(d ^ a, 16);
        c = c + d;
        b = rotr<u64>(b ^ c, 63);
    };

    auto ROUND = [&](usize r) {
        G(r, 0, v[0], v[4], v[8], v[12]);
        G(r, 1, v[1], v[5], v[9], v[13]);
        G(r, 2, v[2], v[6], v[10], v[14]);
        G(r, 3, v[3], v[7], v[11], v[15]);
        G(r, 4, v[0], v[5], v[10], v[15]);
        G(r, 5, v[1], v[6], v[11], v[12]);
        G(r, 6, v[2], v[7], v[8], v[13]);
        G(r, 7, v[3], v[4], v[9], v[14]);
    };

    ROUND(0);
    ROUND(1);
    ROUND(2);
    ROUND(3);
    ROUND(4);
    ROUND(5);
    ROUND(6);
    ROUND(7);
    ROUND(8);
    ROUND(9);
    ROUND(10);
    ROUND(11);

    for (int i = 0; i < 8; i++) {
        _h[i] = _h[i] ^ v[i] ^ v[i + 8];
    }
}

void Blake2b::update(Bytes input) {
    auto [buf, len] = input;

    if (len == 0)
        return;

    usize left = _bufLen;
    usize fill = BLAKE2B_BLOCK_BYTES - left;

    if (len > fill) {
        _bufLen = 0;

        memcpy(_buf + left, buf, fill);
        _incrementCounter(BLAKE2B_BLOCK_BYTES);
        _compress((u64 *)_buf);

        buf += fill;
        len -= fill;

        while (len > BLAKE2B_BLOCK_BYTES) {
            _incrementCounter(BLAKE2B_BLOCK_BYTES);
            _compress((u64 *)buf);

            buf += fill;
            len -= fill;
        }
    }

    memcpy(_buf + _bufLen, buf, len);
    _bufLen += len;
}

Blake2b::Digest Blake2b::digest() {
    _incrementCounter(_bufLen);
    _f[0] = Limits<u64>::MAX;
    memset(_buf + _bufLen, 0, BLAKE2B_BLOCK_BYTES - _bufLen);
    _compress((u64 *)_buf);

    return Digest::from(bytes(_h));
}

Blake2b::Digest blake2b(Bytes input) {
    Blake2b state = {};

    state.init();
    state.update(input);
    return state.digest();
}

} // namespace Karm::Crypto
