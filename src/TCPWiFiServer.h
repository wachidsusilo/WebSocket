#ifndef TCP_WIFI_SERVER_H
#define TCP_WIFI_SERVER_H

#include "TCPServer.h"
#include "TCPWiFiClient.h"

class TCPWiFiServer : public TCPServer {
   public:
    TCPWiFiServer(uint16_t port, uint8_t maxClients = 4)
#ifdef ESP32
        : server(WiFiServer(port, maxClients)) {
        server.setNoDelay(true);
    }
#else
        : server(WiFiServer(port)) {
        server.setNoDelay(true);
    }
#endif

    ~TCPWiFiServer() {
        end();
    }

    void begin() override {
        server.begin();
    }

    std::shared_ptr<TCPClient> accept() override {
        return std::make_shared<TCPWiFiClient>(server.available());
    }

    void end() override {
        server.stop();
    }

   private:
    WiFiServer server;
};

#endif