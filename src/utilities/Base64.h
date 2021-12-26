#ifndef BASE_64_H
#define BASE_64_H

#include "Arduino.h"

class Base64 {
  public:
  Base64(){}
  ~Base64(){}
  String encode(const String &data);
  String encode(const uint8_t* bytes_to_encode, size_t len);
  String decode(const String &encoded_string);
  bool isBase64(uint8_t c);

  private:
  static const String base64_chars;
};

#endif
