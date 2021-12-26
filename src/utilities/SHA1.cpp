#include "SHA1.h"

SHA1::SHA1(const char* text)
  : i(0), n_bits(0) {
  state[0] = 0x67452301;
  state[1] = 0xEFCDAB89;
  state[2] = 0x98BADCFE;
  state[3] = 0x10325476;
  state[4] = 0xC3D2E1F0;
  if (text) add(text);
}

SHA1::SHA1(String str)
  : i(0), n_bits(0) {
  state[0] = 0x67452301;
  state[1] = 0xEFCDAB89;
  state[2] = 0x98BADCFE;
  state[3] = 0x10325476;
  state[4] = 0xC3D2E1F0;
  add(str.c_str());
}

SHA1 &SHA1::add(uint8_t x) {
  add_byte_dont_count_bits(x);
  n_bits += 8;
  return *this;
}

SHA1 &SHA1::add(char c) {
  return add(*(uint8_t*)&c);
}

SHA1 &SHA1::add(const void *data, uint32_t n) {
  if (!data) return *this;
  const uint8_t *ptr = (const uint8_t*)data;
  for (; n && i % sizeof(buf); n--) add(*ptr++);
  for (; n >= sizeof(buf); n -= sizeof(buf)) {
    process_block(ptr);
    ptr += sizeof(buf);
    n_bits += sizeof(buf) * 8;
  }
  for (; n; n--) add(*ptr++);
  return *this;
}

SHA1 &SHA1::add(const char *text) {
  if (!text) return *this;
  return add(text, strlen(text));
}

SHA1 &SHA1::finalize() {
  add_byte_dont_count_bits(0x80);
  while (i % 64 != 56) add_byte_dont_count_bits(0x00);
  for (int j = 7; j >= 0; j--) add_byte_dont_count_bits(n_bits >> j * 8);
  return *this;
}

const SHA1 &SHA1::getHex(char *hex, bool zero_terminate, const char *alphabet) const {
  int k = 0;
  for (int i = 0; i < 5; i++) {
    for (int j = 7; j >= 0; j--) {
      hex[k++] = alphabet[(state[i] >> j * 4) & 0xf];
    }
  }
  if (zero_terminate) hex[k] = '\0';
  return *this;
}

const SHA1 &SHA1::getBase64(char *base64, bool zero_terminate) const {
  static const uint8_t *table = (const uint8_t*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  uint32_t triples[7] = {
    ((state[0] & 0xffffff00) >> 1 * 8),
    ((state[0] & 0x000000ff) << 2 * 8) | ((state[1] & 0xffff0000) >> 2 * 8),
    ((state[1] & 0x0000ffff) << 1 * 8) | ((state[2] & 0xff000000) >> 3 * 8),
    ((state[2] & 0x00ffffff) << 0 * 8),
    ((state[3] & 0xffffff00) >> 1 * 8),
    ((state[3] & 0x000000ff) << 2 * 8) | ((state[4] & 0xffff0000) >> 2 * 8),
    ((state[4] & 0x0000ffff) << 1 * 8),
  };

  for (int i = 0; i < 7; i++) {
    uint32_t x = triples[i];
    base64[i * 4 + 0] = table[(x >> 3 * 6) % 64];
    base64[i * 4 + 1] = table[(x >> 2 * 6) % 64];
    base64[i * 4 + 2] = table[(x >> 1 * 6) % 64];
    base64[i * 4 + 3] = table[(x >> 0 * 6) % 64];
  }

  base64[SHA1_BASE64_SIZE - 2] = '=';
  if (zero_terminate) base64[SHA1_BASE64_SIZE - 1] = '\0';
  return *this;
}

String SHA1::getHexString() {
  char hex[SHA1_HEX_SIZE];
  getHex(hex);
  return String(hex);
}

String SHA1::getBase64String() {
  char base64[SHA1_BASE64_SIZE];
  getBase64(base64);
  return String(base64);
}

void SHA1::add_byte_dont_count_bits(uint8_t x) {
  buf[i++] = x;
  if (i >= sizeof(buf)) {
    i = 0;
    process_block(buf);
  }
}

uint32_t SHA1::rol32(uint32_t x, uint32_t n) {
  return (x << n) | (x >> (32 - n));
}

uint32_t SHA1::make_word(const uint8_t *p) {
  return
    ((uint32_t)p[0] << 3 * 8) |
    ((uint32_t)p[1] << 2 * 8) |
    ((uint32_t)p[2] << 1 * 8) |
    ((uint32_t)p[3] << 0 * 8);
}

void SHA1::shaRound(uint8_t r, uint32_t* w, uint32_t &v, uint32_t &u, uint32_t &x, uint32_t &y, uint32_t &z, uint32_t i) {
  static const uint32_t c0 = 0x5a827999;
  static const uint32_t c1 = 0x6ed9eba1;
  static const uint32_t c2 = 0x8f1bbcdc;
  static const uint32_t c3 = 0xca62c1d6;
  switch (r) {
    case 0:
      z += ((u & (x ^ y)) ^ y) + w[i & 15] + c0 + rol32(v, 5);
      break;
    case 1:
      w[i & 15] = rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
      z += ((u & (x ^ y)) ^ y) + w[i & 15] + c0 + rol32(v, 5);
      break;
    case 2:
      w[i & 15] = rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
      z += (u ^ x ^ y) + w[i & 15] + c1 + rol32(v, 5);
      break;
    case 3:
      w[i & 15] = rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
      z += (((u | x) & y) | (u & x)) + w[i & 15] + c2 + rol32(v, 5);
      break;
    case 4:
      w[i & 15] = rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
      z += (u ^ x ^ y) + w[i & 15] + c3 + rol32(v, 5);
      break;
  }
  u = rol32(u, 30);
}

void SHA1::process_block(const uint8_t *ptr) {
  uint32_t a = state[0];
  uint32_t b = state[1];
  uint32_t c = state[2];
  uint32_t d = state[3];
  uint32_t e = state[4];

  uint32_t w[16];
  for (int _i = 0; _i < 16; _i++) w[_i] = make_word(ptr + _i * 4);

  shaRound(0, w, a, b, c, d, e,  0);
  shaRound(0, w, e, a, b, c, d,  1);
  shaRound(0, w, d, e, a, b, c,  2);
  shaRound(0, w, c, d, e, a, b,  3);
  shaRound(0, w, b, c, d, e, a,  4);
  shaRound(0, w, a, b, c, d, e,  5);
  shaRound(0, w, e, a, b, c, d,  6);
  shaRound(0, w, d, e, a, b, c,  7);
  shaRound(0, w, c, d, e, a, b,  8);
  shaRound(0, w, b, c, d, e, a,  9);
  shaRound(0, w, a, b, c, d, e, 10);
  shaRound(0, w, e, a, b, c, d, 11);
  shaRound(0, w, d, e, a, b, c, 12);
  shaRound(0, w, c, d, e, a, b, 13);
  shaRound(0, w, b, c, d, e, a, 14);
  shaRound(0, w, a, b, c, d, e, 15);
  shaRound(1, w, e, a, b, c, d, 16);
  shaRound(1, w, d, e, a, b, c, 17);
  shaRound(1, w, c, d, e, a, b, 18);
  shaRound(1, w, b, c, d, e, a, 19);
  shaRound(2, w, a, b, c, d, e, 20);
  shaRound(2, w, e, a, b, c, d, 21);
  shaRound(2, w, d, e, a, b, c, 22);
  shaRound(2, w, c, d, e, a, b, 23);
  shaRound(2, w, b, c, d, e, a, 24);
  shaRound(2, w, a, b, c, d, e, 25);
  shaRound(2, w, e, a, b, c, d, 26);
  shaRound(2, w, d, e, a, b, c, 27);
  shaRound(2, w, c, d, e, a, b, 28);
  shaRound(2, w, b, c, d, e, a, 29);
  shaRound(2, w, a, b, c, d, e, 30);
  shaRound(2, w, e, a, b, c, d, 31);
  shaRound(2, w, d, e, a, b, c, 32);
  shaRound(2, w, c, d, e, a, b, 33);
  shaRound(2, w, b, c, d, e, a, 34);
  shaRound(2, w, a, b, c, d, e, 35);
  shaRound(2, w, e, a, b, c, d, 36);
  shaRound(2, w, d, e, a, b, c, 37);
  shaRound(2, w, c, d, e, a, b, 38);
  shaRound(2, w, b, c, d, e, a, 39);
  shaRound(3, w, a, b, c, d, e, 40);
  shaRound(3, w, e, a, b, c, d, 41);
  shaRound(3, w, d, e, a, b, c, 42);
  shaRound(3, w, c, d, e, a, b, 43);
  shaRound(3, w, b, c, d, e, a, 44);
  shaRound(3, w, a, b, c, d, e, 45);
  shaRound(3, w, e, a, b, c, d, 46);
  shaRound(3, w, d, e, a, b, c, 47);
  shaRound(3, w, c, d, e, a, b, 48);
  shaRound(3, w, b, c, d, e, a, 49);
  shaRound(3, w, a, b, c, d, e, 50);
  shaRound(3, w, e, a, b, c, d, 51);
  shaRound(3, w, d, e, a, b, c, 52);
  shaRound(3, w, c, d, e, a, b, 53);
  shaRound(3, w, b, c, d, e, a, 54);
  shaRound(3, w, a, b, c, d, e, 55);
  shaRound(3, w, e, a, b, c, d, 56);
  shaRound(3, w, d, e, a, b, c, 57);
  shaRound(3, w, c, d, e, a, b, 58);
  shaRound(3, w, b, c, d, e, a, 59);
  shaRound(4, w, a, b, c, d, e, 60);
  shaRound(4, w, e, a, b, c, d, 61);
  shaRound(4, w, d, e, a, b, c, 62);
  shaRound(4, w, c, d, e, a, b, 63);
  shaRound(4, w, b, c, d, e, a, 64);
  shaRound(4, w, a, b, c, d, e, 65);
  shaRound(4, w, e, a, b, c, d, 66);
  shaRound(4, w, d, e, a, b, c, 67);
  shaRound(4, w, c, d, e, a, b, 68);
  shaRound(4, w, b, c, d, e, a, 69);
  shaRound(4, w, a, b, c, d, e, 70);
  shaRound(4, w, e, a, b, c, d, 71);
  shaRound(4, w, d, e, a, b, c, 72);
  shaRound(4, w, c, d, e, a, b, 73);
  shaRound(4, w, b, c, d, e, a, 74);
  shaRound(4, w, a, b, c, d, e, 75);
  shaRound(4, w, e, a, b, c, d, 76);
  shaRound(4, w, d, e, a, b, c, 77);
  shaRound(4, w, c, d, e, a, b, 78);
  shaRound(4, w, b, c, d, e, a, 79);

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}
