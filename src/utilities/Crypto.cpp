#include "Crypto.h"

String Crypto::generateHandshakeKey(String key) {
    char base64[SHA1_BASE64_SIZE];
    SHA1(key)
        .add("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")
        .finalize()
        .getBase64(base64);
    return String(base64);
}

String Crypto::randomBytes(size_t len) {
    static const char data[] = "0123456789abcdefABCDEFGHIJKLMNOPQRSTUVEXYZ";
    srand(millis());
    String result;
    for (size_t i = 0; i < len; i++) {
        result += data[rand() % 42];
    }
    return result;
}

String Crypto::getBit(uint64_t data, size_t len) {
    String str;
    for (int i = len - 1; i >= 0; i--) {
        str += uint64ToString(1 & (data >> i));
    }
    return str;
}

String Crypto::uint64ToString(uint64_t input) {
    String result = "";
    uint8_t base = 10;
    do {
        char c = input % base;
        input /= base;
        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        result = c + result;
    } while (input);
    return result;
}

uint16_t Crypto::swapEndianness(uint16_t num) {
    return (num >> 8) | (num << 8);
}

void Crypto::remaskData(String& data, uint8_t maskingKey[4]) {
    for (int i = 0; i < data.length(); i++) {
        data.setCharAt(i, data[i] ^ maskingKey[i % 4]);
    }
}

bool Crypto::shouldAddDefaultHeader(const String& keyWord, const std::vector<std::pair<String, String>>& customHeaders) {
    for (const auto& header : customHeaders) {
        if (keyWord.equals(header.first)) {
            return false;
        }
    }
    return true;
}

Crypto::HandshakeRequestResult Crypto::generateHandshake(const String& host, const String& uri, const std::vector<std::pair<String, String>>& customHeaders) {
    Crypto crypto;
    Base64 base64;
    String key = base64.encode(crypto.randomBytes(16));
    String handshake = "GET " + uri + " HTTP/1.1\r\n";
    handshake += "Host: " + host + "\r\n";
    handshake += "Sec-WebSocket-Key: " + key + "\r\n";

    for (const auto& header : customHeaders) {
        handshake += header.first + ": " + header.second + "\r\n";
    }

    if (shouldAddDefaultHeader("Upgrade", customHeaders)) {
        handshake += "Upgrade: websocket\r\n";
    }

    if (shouldAddDefaultHeader("Connection", customHeaders)) {
        handshake += "Connection: Upgrade\r\n";
    }

    if (shouldAddDefaultHeader("Sec-WebSocket-Version", customHeaders)) {
        handshake += "Sec-WebSocket-Version: 13\r\n";
    }

    if (shouldAddDefaultHeader("User-Agent", customHeaders)) {
        handshake += "User-Agent: ESP32\r\n";
    }

    if (shouldAddDefaultHeader("Origin", customHeaders)) {
        handshake += "Origin: https://codedillo.com\r\n";
    }

    handshake += "\r\n";
    Crypto::HandshakeRequestResult result;
    result.requestStr = handshake;
    result.expectedAcceptKey = crypto.generateHandshakeKey(key);
    return result;
}

Crypto::HandshakeResponseResult Crypto::parseHandshakeResponse(std::vector<String> responseHeaders) {
    bool didUpgradeToWebsockets = false;
    bool isConnectionUpgraded = false;
    String serverAccept = "";

    for (String header : responseHeaders) {
        int colonIndex = header.indexOf(':');
        String key = header.substring(0, colonIndex);
        String value = header.substring(colonIndex + 1);
        key.trim();
        value.trim();
        key.toLowerCase();

        if (key.equals("upgrade")) {
            value.toLowerCase();
            didUpgradeToWebsockets = value.equals("websocket");
        } else if (key.equals("connection")) {
            value.toLowerCase();
            isConnectionUpgraded = value.equals("upgrade");
        } else if (key.equals("sec-websocket-accept")) {
            serverAccept = value;
        }
    }

    Crypto::HandshakeResponseResult result;
    result.isSuccess = serverAccept != "" && didUpgradeToWebsockets && isConnectionUpgraded;
    result.serverAccept = serverAccept;
    return result;
}

Crypto::HandshakeServerResult Crypto::parseHandshakeRequest(std::vector<String> requestHeaders) {
    bool isUpgrade = false;
    bool isConnection = false;
    bool isSecWebSocketKey = false;
    bool isSecWebSocketVersion = false;
    String handshakeKey;

    for (String header : requestHeaders) {
        int colonIndex = header.indexOf(':');
        String key = header.substring(0, colonIndex);
        String value = header.substring(colonIndex + 1);
        key.trim();
        value.trim();
        key.toLowerCase();

        if (key.equals("connection")) {
            value.toLowerCase();
            isConnection = value.equals("upgrade");
        } else if (key.equals("upgrade")) {
            value.toLowerCase();
            isUpgrade = value.equals("websocket");
        } else if (key.equals("sec-websocket-version")) {
            isSecWebSocketVersion = value.equals("13");
        } else if (key.equals("sec-websocket-key")) {
            isSecWebSocketKey = !value.isEmpty();
            handshakeKey = value;
        }
    }

    Crypto::HandshakeServerResult result;
    result.isValid = isUpgrade && isConnection && isSecWebSocketKey && isSecWebSocketVersion;
    result.key = generateHandshakeKey(handshakeKey);
    return result;
}
