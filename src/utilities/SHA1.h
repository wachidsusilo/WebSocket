#ifndef SHA_1_H
#define SHA_1_H

#include "Arduino.h"

#define SHA1_HEX_SIZE (40 + 1)
#define SHA1_BASE64_SIZE (28 + 1)

class SHA1 {
  public:
    SHA1(const char* text = NULL);
    SHA1(String str);
    SHA1 &add(uint8_t x);
    SHA1 &add(char c);
    SHA1 &add(const void *data, uint32_t n);
    SHA1 &add(const char *text);
    SHA1 &finalize();
    const SHA1 &getHex(char *hex, bool zero_terminate = true, const char *alphabet = "0123456789abcdef") const;
    const SHA1 &getBase64(char *base64, bool zero_terminate = true) const;
    String getHexString();
    String getBase64String();

  private:
    uint32_t state[5];
    uint8_t buf[64];
    uint32_t i;
    uint64_t n_bits;
    void add_byte_dont_count_bits(uint8_t x);
    void process_block(const uint8_t *ptr);
    uint32_t rol32(uint32_t x, uint32_t n);
    uint32_t make_word(const uint8_t *p);
    void shaRound(uint8_t r, uint32_t* w, uint32_t &v, uint32_t &u, uint32_t &x, uint32_t &y, uint32_t &z, uint32_t i);
};

#endif
