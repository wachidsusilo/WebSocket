#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "Arduino.h"
#include "IPAddress.h"
#include "utilities/Crypto.h"

class TCPClient {
   public:
    virtual size_t write(uint8_t* data, size_t len) = 0;
    virtual int read(uint8_t* buffer, size_t len) = 0;
    virtual int available() = 0;
    virtual int connected() = 0;
    virtual IPAddress remoteIP() = 0;
    virtual uint16_t remotePort() = 0;

   protected:
    virtual bool connect(const String& host, const uint16_t& port, const String& path, std::vector<std::pair<String, String>> customHeaders) = 0;
    virtual void disconnect() = 0;

   public:
    bool begin(const String& host, const uint16_t& port, const String& path, std::vector<std::pair<String, String>> customHeaders) {
        if (!connect(host, port, path, customHeaders)) return false;
        Crypto::HandshakeRequestResult handshake = Crypto::generateHandshake(host, path, customHeaders);

        write(handshake.requestStr);
        if (!connected()) return false;
        String header = readLine();

        if (!header.startsWith("HTTP/1.1 101")) {
            disconnect();
            return false;
        }

        std::vector<String> serverResponseHeaders;
        String line;
        while (available()) {
            line = readLine();
            line.trim();
            serverResponseHeaders.push_back(line);
            if (!line.length()) break;
        }

        Crypto::HandshakeResponseResult parsedResponse = Crypto::parseHandshakeResponse(serverResponseHeaders);
        bool serverAcceptMismatch = !parsedResponse.serverAccept.equals(handshake.expectedAcceptKey);
        if (parsedResponse.isSuccess == false || serverAcceptMismatch) {
            disconnect();
            return false;
        }
        return true;
    }

    void end() {
        disconnect();
    }

    size_t write(const String& data) {
        return write((uint8_t*)data.c_str(), data.length());
    }

    int read() {
        uint32_t lastMillis = millis();
        do {
            uint8_t data = 0;
            if (read(&data, 1) > 0) return data;
        } while (millis() - lastMillis < 1000);
        return -1;
    }

    String readString() {
        String ret;
        int c = read();
        while (c >= 0) {
            ret += (char)c;
            c = read();
        }
        return ret;
    }

    String readLine() {
        String ret;
        int c = read();
        while (c >= 0 && c != '\n') {
            ret += (char)c;
            c = read();
        }
        ret.trim();
        return ret;
    }
};

#endif
