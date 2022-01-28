#include "WSClient.h"

WSClient::WSClient()
    : client(std::make_shared<TCPWiFiClient>()), state(Closed) {
    reshuffleMask();
}

WSClient::WSClient(std::shared_ptr<TCPClient> client)
    : client(client), rmtIP(client->remoteIP()), rmtPort(client->remotePort()), state(client && client->connected() ? Connected : Closed) {
    reshuffleMask();
}

WSClient::~WSClient() {
#ifdef ESP32
    if (handler) vTaskDelete(handler);
#endif
}

void WSClient::addHeader(const String& key, const String& value) {
    customHeaders.push_back({key, value});
}

bool WSClient::begin(String url) {
    if (!client) {
        if (errorCallback) errorCallback(*this, "Client is not initialized");
        return false;
    }

    if (url.startsWith("ws://")) {
        url.replace("ws://", "");
    } else if (url.startsWith("wss://")) {
        url.replace("wss://", "");
    } else {
        if (errorCallback) errorCallback(*this, "Invalid URL");
        return false;
    }
    url.trim();

    int pathIdx = url.indexOf("/");
    if (pathIdx > 0) {
        path = url.substring(pathIdx);
        url = url.substring(0, pathIdx);
    } else {
        path = "/";
    }

    int portIdx = url.indexOf(":");
    if (portIdx > 0) {
        port = url.substring(portIdx + 1).toInt();
        host = url.substring(0, portIdx);
    } else {
        port = 80;
        host = url;
    }

    _close();

    state = Connecting;
    if (client->begin(host, port, path, customHeaders)) {
        if (openCallback) openCallback(*this);
        state = Connected;
#ifdef ESP32
        if (!handler) xTaskCreate(pollingTask, "pollingTask", 2 * 8192, this, 1, &handler);
#endif
        rmtIP = client->remoteIP();
        rmtPort = client->remotePort();
        return true;
    }
    if (errorCallback) errorCallback(*this, "Cannot connect to " + url);
    state = Closed;
#ifdef ESP32
    if (!handler) xTaskCreate(pollingTask, "pollingTask", 2 * 8192, this, 1, &handler);
#endif
    return false;
}

bool WSClient::send(String data) {
    if (!client) return false;
    if (data.length() > 65535) return false;
    if (useMask) Crypto::remaskData(data, maskingKey);
    uint16_t len = data.length();
    Frame::Header header(1, 0, useMask ? 1 : 0, Frame::Text, len);
    uint16_t bin = header.getBinary();
    uint16_t extendedPayload = header.getExtendedPayload();
    uint8_t payload[len > 125 ? len + (useMask ? 8 : 4) : len + (useMask ? 6 : 2)];
    memcpy(payload, (uint8_t*)&bin, 2);
    if (len > 125) {
        memcpy(payload + 2, (uint8_t*)&extendedPayload, 2);
        if (useMask) memcpy(payload + 4, maskingKey, 4);
        memcpy(payload + (useMask ? 8 : 4), (uint8_t*)data.c_str(), len);
    } else {
        if (useMask) memcpy(payload + 2, maskingKey, 4);
        memcpy(payload + (useMask ? 6 : 2), (uint8_t*)data.c_str(), len);
    }
    if (useMask) reshuffleMask();
    return client->write(payload, sizeof(payload));
}

bool WSClient::ping(String data) {
    if (!client) return false;
    uint16_t len = data.length();
    if (useMask) Crypto::remaskData(data, maskingKey);
    Frame::Header header(1, 0, useMask ? 1 : 0, Frame::Ping, len);
    uint16_t bin = header.getBinary();
    uint8_t payload[len + (useMask ? 6 : 2)];
    memcpy(payload, (uint8_t*)&bin, 2);
    if (useMask) memcpy(payload + 2, maskingKey, 4);
    memcpy(payload + (useMask ? 6 : 2), (uint8_t*)data.c_str(), len);
    if (useMask) reshuffleMask();
    return client->write(payload, sizeof(payload));
}

bool WSClient::pong(String data) {
    if (!client) return false;
    uint16_t len = data.length();
    if (useMask) Crypto::remaskData(data, maskingKey);
    Frame::Header header(1, 0, useMask ? 1 : 0, Frame::Pong, len);
    uint16_t bin = header.getBinary();
    uint8_t payload[len + (useMask ? 6 : 2)];
    memcpy(payload, (uint8_t*)&bin, 2);
    if (useMask) memcpy(payload + 2, maskingKey, 4);
    memcpy(payload + (useMask ? 6 : 2), (uint8_t*)data.c_str(), len);
    if (useMask) reshuffleMask();
    return client->write(payload, sizeof(payload));
}

bool WSClient::close(CloseReason code, String reason) {
#ifdef ESP32
    if (handler) vTaskDelete(handler);
#endif
    return _close(code, reason);
}

bool WSClient::_close(CloseReason code, String reason) {
    if (!client || state != Connected) return false;
    state = Closed;
    String data(char((uint16_t)code >> 8) + String(char((uint16_t)code)) + (reason.length() > 0 ? reason : getReason(code)));
    if (useMask) Crypto::remaskData(data, maskingKey);
    uint16_t len = data.length();
    Frame::Header header(1, 0, useMask ? 1 : 0, Frame::Close, len);
    uint16_t bin = header.getBinary();
    uint8_t payload[len + (useMask ? 6 : 2)];
    memcpy(payload, (uint8_t*)&bin, 2);
    if (useMask) memcpy(payload + 2, maskingKey, 4);
    memcpy(payload + (useMask ? 6 : 2), (uint8_t*)data.c_str(), len);
    int res = client->write(payload, sizeof(payload));
    client->end();
    if (closeCallback) closeCallback(*this, String((uint16_t)code) + " -> " + getReason(code) + (reason.length() > 0 ? ": " + reason : ""));
    if (useMask) reshuffleMask();
    return res;
}

bool WSClient::isConnected() {
    if (!client) return false;
    return client->connected();
}

bool WSClient::reconnect() {
    if (!client) return false;
    if (state == Connecting) return false;
    state = Connecting;
    if (client->begin(host, port, path, customHeaders)) {
        if (openCallback) openCallback(*this);
        rmtIP = client->remoteIP();
        rmtPort = client->remotePort();
        state = Connected;
        return true;
    }
    state = Closed;
    if (errorCallback) errorCallback(*this, "Reconnection failed");
    return false;
}

void WSClient::poll() {
    if (!client || !client->available()) return;
    uint16_t bin = 0;
    int i = 0;
    while (isConnected()) {
        int res = client->read();
        if (res >= 0) bin |= ((uint8_t)res) << i++ * 8;
        if (i > 1) break;
    }

    Frame::Header header(bin);
    uint16_t payloadLen = header.payload;
    if ((useMask && header.mask) || (!useMask && !header.mask) || !Frame::isValid(Frame::Opcode(header.opcode))) {
        _close(CloseReason_ProtocolError);
        return;
    }
    if (header.opcode == 0) {
        _close(CloseReason_UnsupportedData);
        return;
    }
    if (header.payload == 127) {
        _close(CloseReason_MessageTooBig);
        return;
    }
    if (header.payload == 126) {
        bin = 0;
        i = 0;
        while (isConnected()) {
            int res = client->read();
            if (res >= 0) bin |= ((uint8_t)res) << i++ * 8;
            if (i > 1) break;
        }
        payloadLen = Crypto::swapEndianness(bin);
    }

    uint8_t maskingKey[4];
    if (header.mask) {
        i = 0;
        while (isConnected()) {
            int res = client->read();
            if (res >= 0) maskingKey[i++] = (uint8_t)res;
            if (i > 3) break;
        }
        if (i != 4) {
            _close(CloseReason_ProtocolError);
            return;
        }
    }

    char payload[payloadLen + 1];
    payload[payloadLen] = 0;
    i = 0;
    if (payloadLen > 0) {
        while (isConnected()) {
            int res = client->read();
            if (res > 0) payload[i++] = res;
            if (i >= payloadLen) break;
        }
    }

    String payloadData(payload);
    if (header.mask) Crypto::remaskData(payloadData, maskingKey);
    switch (header.opcode) {
        case Frame::Text:
        case Frame::Binary:
            if (messageCallback) messageCallback(*this, payloadData);
            break;
        case Frame::Ping:
            pong(payloadData);
            if (pingCallback) pingCallback(*this, payloadData);
            break;
        case Frame::Pong:
            if (pongCallback) pongCallback(*this, payloadData);
            break;
        case Frame::Close:
            uint16_t closeReason = CloseReason_NormalClosure;
            if (payloadLen >= 2) {
                closeReason = (payloadData[0] << 8) + payloadData[1];
                closeReason = Crypto::swapEndianness(closeReason);
            }
            _close(static_cast<CloseReason>(closeReason));
            break;
    }
}

void WSClient::setUseMask(bool useMask) {
    this->useMask = useMask;
}

void WSClient::onOpen(EmptyCallback callback) {
    openCallback = callback;
}

void WSClient::onClose(StringCallback callback) {
    closeCallback = callback;
}

void WSClient::onMessage(StringCallback callback) {
    messageCallback = callback;
}

void WSClient::onPing(StringCallback callback) {
    pingCallback = callback;
}

void WSClient::onPong(StringCallback callback) {
    pongCallback = callback;
}

void WSClient::onError(StringCallback callback) {
    errorCallback = callback;
}

IPAddress WSClient::remoteIP() {
    return rmtIP;
}

uint16_t WSClient::remotePort() {
    return rmtPort;
}

String WSClient::getReason(CloseReason reason) {
    switch (reason) {
        case CloseReason_NormalClosure:
            return "Normal Closure";
        case CloseReason_GoingAway:
            return "Going Away";
        case CloseReason_ProtocolError:
            return "Protocol Error";
        case CloseReason_UnsupportedData:
            return "Unsupported Data";
        case CloseReason_NoStatusRcvd:
            return "No Status Received";
        case CloseReason_AbnormalClosure:
            return "Abnormal Closure";
        case CloseReason_InvalidPayloadData:
            return "Invalid Payload Data";
        case CloseReason_PolicyViolation:
            return "Policy Violation";
        case CloseReason_MessageTooBig:
            return "Message Too Big";
        case CloseReason_InternalServerError:
            return "Internal Server Error";
        default:
            return "No Reason";
    }
}

void WSClient::reshuffleMask() {
    for (int i = 0; i < 4; i++) maskingKey[i] = random(256);
}

void WSClient::run() {
    if (isConnected()) {
        if (state == Connected) poll();
    } else if (millis() - lastReconnectAttempt > 15000) {
        lastReconnectAttempt = millis();
        if (state == Connected) _close(CloseReason_InternalServerError);
        reconnect();
    }
}

#ifdef ESP32
void WSClient::pollingTask(void* ptr) {
    WSClient* client = (WSClient*)ptr;
    while (true) {
        delay(2);
        if (!client) vTaskDelete(NULL);
        client->run();
    }
}
#endif
