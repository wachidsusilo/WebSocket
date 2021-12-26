#ifndef FRAME_H
#define FRAME_H

#include "Arduino.h"
#include "Crypto.h"

class Frame {
  public:
    enum Opcode {
      Text = 0x1,
      Binary = 0x2,
      Close = 0x8,
      Ping = 0x9,
      Pong = 0xA
    };

    struct Header {
      uint8_t fin : 1;
      uint8_t flags : 3;
      uint8_t mask : 1;
      uint8_t opcode : 4;
      uint8_t payload : 7;
      uint64_t extendedPayload;

      Header(uint16_t data = 0, uint64_t extendedPayload = 0);
      Header(uint8_t fin, uint8_t flags, uint8_t mask, uint8_t opcode, uint64_t len);

      uint16_t getBinary();
      uint64_t getExtendedPayload();
      String getBinarySequence(String delimiter = "");
    };

    static bool isValid(Opcode opcode);
};

#endif
