#include "Frame.h"

bool Frame::isValid(Frame::Opcode opcode){
  return opcode == 0 || opcode == Frame::Text || opcode == Frame::Binary || opcode == Frame::Close || opcode == Frame::Ping || opcode == Frame::Pong;
}

Frame::Header::Header(uint16_t data, uint64_t extendedPayload) {
  data    = Crypto::swapEndianness(data);
  fin     = (data >> 15) & 0x1;
  flags   = (data >> 12) & 0x8;
  opcode  = (data >> 8) & 0xF;
  mask    = (data >> 7) & 0x1;
  payload = (data & 0x7F);
  
  if (payload < 126) {
    this->extendedPayload = 0;
  } else if (payload == 126) {
    this->extendedPayload = Crypto::swapEndianness((uint16_t)extendedPayload);
  } else {
    this->extendedPayload = Crypto::swapEndianness(extendedPayload);
  }
}

Frame::Header::Header(uint8_t fin, uint8_t flags, uint8_t mask, uint8_t opcode, uint64_t len)
  : fin(fin), flags(flags), mask(mask), opcode(opcode), payload(0) {
  if (len < 126) {
    payload = len;
    extendedPayload = 0;
  } else if (len < 65536) {
    payload = 126;
    extendedPayload = len;
  } else {
    payload = 127;
    extendedPayload = len;
  }
}

uint16_t Frame::Header::getBinary() {
  uint16_t ret = 0;
  ret |= fin << 15;
  ret |= flags << 12;
  ret |= opcode << 8;
  ret |= mask << 7;
  ret |= payload;
  return Crypto::swapEndianness(ret);
}

uint64_t Frame::Header::getExtendedPayload() {
  if (payload < 126) {
    return 0;
  } else if (payload == 126) {
    return Crypto::swapEndianness((uint16_t)extendedPayload);
  } else {
    return Crypto::swapEndianness(extendedPayload);
  }
}

String Frame::Header::getBinarySequence(String delimiter) {
  String str = Crypto::getBit(fin, 1)
               + delimiter + Crypto::getBit(flags, 3)
               + delimiter + Crypto::getBit(mask, 1)
               + delimiter + Crypto::getBit(opcode, 4)
               + delimiter + Crypto::getBit(payload, 7);

  if (payload == 126) {
    str += delimiter + Crypto::getBit(extendedPayload, 16);
  } else if (payload == 127) {
    str += delimiter + Crypto::getBit(extendedPayload, 64);
  }
  return str;
}
