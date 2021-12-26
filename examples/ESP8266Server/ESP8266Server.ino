#include "ESP8266WiFi.h"
#include "WSServer.h"

WSServer server(80);

void onConnection(WSClient& ws) {
    Serial.println("Client Connected: " + ws.remoteIP().toString() + ":" + String(ws.remotePort()));
    ws.onClose([](WSClient& c, String reason) {
        Serial.println("Client Disconnected: " + c.remoteIP().toString() + ":" + String(c.remotePort()) + " - " + reason);
    });
    ws.onMessage([](WSClient& c, String data) {
        c.send(data);
        Serial.println("Client Message: " + c.remoteIP().toString() + ":" + String(c.remotePort()) + " - " + data);
    });
    ws.onPing([](WSClient& c, String data) {
        Serial.println("Client Ping: " + c.remoteIP().toString() + ":" + String(c.remotePort()) + " - " + data);
    });
    ws.onPong([](WSClient& c, String data) {
        Serial.println("Client Pong: " + c.remoteIP().toString() + ":" + String(c.remotePort()) + " - " + data);
    });
    ws.onError([](WSClient& c, String data) {
        Serial.println("Client Error: " + c.remoteIP().toString() + ":" + String(c.remotePort()) + " - " + data);
    });
}

void setup() {
    Serial.begin(115200);

    WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
    while (!WiFi.isConnected()) {
        delay(10);
    }
    Serial.println("WiFi Connected");
    delay(1000);

    server.onConnection(onConnection);
    server.begin();
}

void loop() {
    //IMPORTANT: Call this function in your loop() function
    //For ESP8266 or Arduino only.
    //On ESP32 this function is not necessary.
    server.run();

    if (Serial.available()) {
        String data = Serial.readString();
        for (auto& c : server.getClients()) {
            c.send(data);
        }
    }
}
