#include "WSServer.h"

WSServer::WSServer(uint16_t port, uint8_t maxClients)
    : server(std::make_shared<TCPWiFiServer>(port, maxClients)) {}

WSServer::WSServer(std::shared_ptr<TCPServer> server)
    : server(server) {}

WSServer::~WSServer() {
    end();
#ifdef ESP32
    if (handler) vTaskDelete(handler);
#endif
}

void WSServer::begin() {
    if (!server) return;
    server->begin();
#ifdef ESP32
    if (!handler) xTaskCreate(pollingTask, "serverTask", 2 * 8192, this, 1, &handler);
#endif
}

void WSServer::end() {
    if (!server) return;
    server->end();
}

std::vector<WSClient>& WSServer::getClients() {
    return clients;
}

void WSServer::onConnection(WSCallback callback) {
    this->callback = callback;
}

void WSServer::poll() {
    for (auto& client : clients) {
        client.poll();
    }
}

void WSServer::close(String id) {
    for (auto& client : clients) {
        if (client.id == id) {
            client.close(CloseReason_AbnormalClosure);
        }
    }
}

bool WSServer::hasClients() {
    return !clients.empty();
}

bool WSServer::hasClient(String id) {
    for (auto& client : clients) {
        if (client.id == id) return true;
    }
    return false;
}

void WSServer::cleanup() {
    for (int i = 0; i < clients.size(); i++) {
        if (!clients[i].isConnected()) {
            clients[i].close(CloseReason_AbnormalClosure);
            clients.erase(clients.begin() + i);
            i--;
        }
    }
}

void WSServer::accept() {
    if (!server) return;
    std::shared_ptr<TCPClient> client = server->accept();
    if (!client) return;
    for (auto& c : clients) {
        if (c.remoteIP() == client->remoteIP() && c.remotePort() == client->remotePort()) return;
    }
    if (!client->connected()) return;

    std::vector<String> requestHeaders;
    String line;
    while (client->available()) {
        line = client->readLine();
        line.trim();
        requestHeaders.push_back(line);
        if (!line.length()) break;
    }

    Crypto::HandshakeServerResult result = Crypto::parseHandshakeRequest(requestHeaders);
    if (!result.isValid) {
        client->end();
        return;
    }

    String response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Connection: Upgrade\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Sec-WebSocket-Version: 13\r\n";
    response += "Sec-WebSocket-Version: 13\r\n";
    response += "Sec-WebSocket-Accept: " + result.key + "\r\n\r\n";

    client->write(response);

    WSClient wsClient(client);
    wsClient.id = generateId();
    wsClient.setUseMask(false);
    if (callback) callback(wsClient);
    clients.push_back(wsClient);
}

String WSServer::generateId() {
    String id = "";
    for (int i = 0; i < 16; i++) {
        id += String(random(0, 16), HEX);
    }
    return id;
}

void WSServer::run() {
    poll();
    if (millis() - lastAccept > 1000) {
        lastAccept = millis();
        accept();
    }
    if (millis() - lastCleanup > 5000) {
        lastCleanup = millis();
        cleanup();
    }
}

#ifdef ESP32
void WSServer::pollingTask(void* ptr) {
    WSServer* server = (WSServer*)ptr;
    while (true) {
        delay(2);
        if (!server) vTaskDelete(NULL);
        server->run();
    }
}
#endif