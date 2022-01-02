#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <functional>
#include <memory>

#include "Arduino.h"
#include "TCPWiFiClient.h"
#include "utilities/Frame.h"

enum CloseReason {
    CloseReason_None = -1,
    CloseReason_NormalClosure = 1000,
    CloseReason_GoingAway = 1001,
    CloseReason_ProtocolError = 1002,
    CloseReason_UnsupportedData = 1003,
    CloseReason_NoStatusRcvd = 1005,
    CloseReason_AbnormalClosure = 1006,
    CloseReason_InvalidPayloadData = 1007,
    CloseReason_PolicyViolation = 1008,
    CloseReason_MessageTooBig = 1009,
    CloseReason_InternalServerError = 1011
};

class WSClient {
   public:
    using EmptyCallback = std::function<void(WSClient&)>;
    using StringCallback = std::function<void(WSClient&, String)>;
    String id;

    WSClient();
    WSClient(std::shared_ptr<TCPClient> client);
    ~WSClient();

    void addHeader(const String &key, const String &value);
    bool begin(String url);
    bool send(String data);
    bool ping(String data = "");
    bool pong(String data = "");
    bool close(CloseReason code = CloseReason_GoingAway, String reason = "");
    bool isConnected();
    bool reconnect();
    void setUseMask(bool useMask);
    void poll();
    void onOpen(EmptyCallback callback);
    void onClose(StringCallback callback);
    void onMessage(StringCallback callback);
    void onPing(StringCallback callback);
    void onPong(StringCallback callback);
    void onError(StringCallback callback);
    void run();
    IPAddress remoteIP();
    uint16_t remotePort();

   private:
    enum State {
        Connecting,
        Connected,
        Closed
    };

    std::shared_ptr<TCPClient> client;
    String host;
    String path;
    uint16_t port;
    uint8_t maskingKey[4];
    State state = Closed;
    bool useMask = true;
    IPAddress rmtIP;
    uint16_t rmtPort;
    uint32_t lastReconnectAttempt = 0;

    EmptyCallback openCallback = NULL;
    StringCallback closeCallback = NULL;
    StringCallback messageCallback = NULL;
    StringCallback pingCallback = NULL;
    StringCallback pongCallback = NULL;
    StringCallback errorCallback = NULL;

    std::vector<std::pair<String, String>> customHeaders;
    String getReason(CloseReason reason);
    bool _close(CloseReason code = CloseReason_GoingAway, String reason = "");
    void reshuffleMask();
#ifdef ESP32
    TaskHandle_t handler = NULL;
    static void pollingTask(void *ptr);
#endif
};

#endif
