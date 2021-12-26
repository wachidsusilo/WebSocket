#ifndef CRYPTO_H
#define CRYPTO_H

#include "Arduino.h"
#include "Base64.h"
#include "SHA1.h"
#include "vector"

class Crypto {
   public:
    struct HandshakeRequestResult {
        String requestStr;
        String expectedAcceptKey;
    };

    struct HandshakeResponseResult {
        bool isSuccess;
        String serverAccept;
    };

    struct HandshakeServerResult {
        bool isValid;
        String key;
    };

    static String generateHandshakeKey(String key);
    static String randomBytes(size_t len);
    static String getBit(uint64_t data, size_t len);
    static String uint64ToString(uint64_t input);
    static uint16_t swapEndianness(uint16_t num);
    static void remaskData(String& data, uint8_t maskingKey[4]);

    static bool shouldAddDefaultHeader(const String& keyWord, const std::vector<std::pair<String, String>>& customHeaders);
    static HandshakeRequestResult generateHandshake(const String& host, const String& uri, const std::vector<std::pair<String, String>>& customHeaders);
    static HandshakeResponseResult parseHandshakeResponse(std::vector<String> responseHeaders);
    static HandshakeServerResult parseHandshakeRequest(std::vector<String> requestHeaders);
};

#endif
