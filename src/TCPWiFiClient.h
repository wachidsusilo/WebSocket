#ifndef TCP_WIFI_CLIENT_H
#define TCP_WIFI_CLIENT_H

#include "TCPClient.h"
#ifdef ESP32
#include "WiFi.h"
#elif defined(ESP8266)
#include "ESP8266WiFi.h"
#else
#error "Unsupported platform. By default, only ESP32 and ESP8266 are supported."
#endif

class TCPWiFiClient : public TCPClient {
   public:
    TCPWiFiClient() : client(WiFiClient()) {}
    TCPWiFiClient(WiFiClient client) : client(client) {}

    ~TCPWiFiClient() {
        disconnect();
    }

    bool connect(const String &host, const uint16_t &port, const String &path, std::vector<std::pair<String, String>> customHeaders) override {
        if (!WiFi.isConnected()) return false;
        return client.connect(host.c_str(), port);
    }

    size_t write(uint8_t *data, size_t len) override {
        if (connected()) return client.write(data, len);
        return 0;
    }

    int read(uint8_t *buffer, size_t len) override {
        if (available()) return client.read(buffer, len);
        return -1;
    }

    int available() override {
        return client.available();
    }

    int connected() override {
        return client.connected();
    }

    IPAddress remoteIP() override {
        return client.remoteIP();
    }

    uint16_t remotePort() override {
        return client.remotePort();
    }

    void disconnect() override {
        client.stop();
    }

   private:
    WiFiClient client;
};

#endif