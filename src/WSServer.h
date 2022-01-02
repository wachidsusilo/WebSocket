#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <functional>
#include <memory>

#include "TCPWiFiServer.h"
#include "WSClient.h"

class WSServer {
   public:
    using WSCallback = std::function<void(WSClient&)>;

    WSServer(uint16_t port = 80, uint8_t maxClients = 4);
    WSServer(std::shared_ptr<TCPServer> server);
    ~WSServer();
    void begin();
    void poll();
    void end();
    void run();
    bool hasClients();
    bool hasClient(String id);
    std::vector<WSClient>& getClients();
    void onConnection(WSCallback callback);

    WSServer(const WSServer&) = delete;
    WSServer(WSServer&&) = delete;
    WSServer& operator=(const WSServer&) = delete;

   private:
    std::shared_ptr<TCPServer> server;
    std::vector<WSClient> clients;
    WSCallback callback = NULL;
    uint32_t lastAccept = 0;
    uint32_t lastCleanup = 0;
    String generateId();
    void accept();
    void cleanup();
#ifdef ESP32
    TaskHandle_t handler = NULL;
    static void pollingTask(void* ptr);
#endif
};

#endif