## WebSocket
Simple WebSocket Library for ESP8266 and ESP32. This library can support any network interface. If you are not using `WiFi`, you have to create a class which is inheriting `TCPClient` and `TCPServer`. Please note that you need to `override` the pure virtual methods. You can look into `TCPWiFiClient.h` and `TCPWiFiServer.h` as an example.

## Examples
All of these examples are available in the examples directory. For ESP8266, you need to poll the event in your `void loop()` routine. On the ESP32, this event polling is done using `FreeRTOS` task.

### 1. ESP8266 as Client
````c++
#include "ESP8266WiFi.h"
#include "WSClient.h"

WSClient client;
String url = "ws://10.0.0.2:5000";

void onOpen(WSClient&) {
    Serial.println("onOpen: Connected to " + url);
}

void onClose(WSClient&, String reason) {
    Serial.println("onClose: " + reason);
}

void onMessage(WSClient&, String data) {
    Serial.println("onMessage: " + data);
}

void onPing(WSClient&, String data) {
    Serial.println("onPing: " + data);
}

void onPong(WSClient&, String data) {
    Serial.println("onPong: " + data);
}

void onError(WSClient&, String data) {
    Serial.println("onError: " + data);
}

void setup() {
    Serial.begin(115200);

    WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
    while (!WiFi.isConnected()) {
        delay(10);
    }
    Serial.println("WiFi Connected");
    delay(1000);

    client.onOpen(onOpen);
    client.onClose(onClose);
    client.onMessage(onMessage);
    client.onPing(onPing);
    client.onPong(onPong);
    client.onError(onError);
    client.begin(url);
}

void loop() {
    //IMPORTANT: Call this function in your loop() function
    //For ESP8266 or Arduino only.
    //On ESP32 this function is not necessary.
    client.run();

    if (Serial.available()) {
        String data = Serial.readString();
        if (data.equals("close")) {
            client.close();
        } else if (data.equals("open")) {
            client.begin(url);
        } else {
            client.send(data);
        }
    }
}
````

### 2. ESP8266 as Server
````c++
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
````

### 2. ESP32 as Client
````c++
#include "WSClient.h"
#include "WiFi.h"

WSClient client;
String url = "ws://10.0.0.2:5000";

void onOpen(WSClient&) {
    Serial.println("onOpen: Connected to " + url);
}

void onClose(WSClient&, String reason) {
    Serial.println("onClose: " + reason);
}

void onMessage(WSClient&, String data) {
    Serial.println("onMessage: " + data);
}

void onPing(WSClient&, String data) {
    Serial.println("onPing: " + data);
}

void onPong(WSClient&, String data) {
    Serial.println("onPong: " + data);
}

void onError(WSClient&, String data) {
    Serial.println("onError: " + data);
}

void setup() {
    Serial.begin(115200);

    WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
    while (!WiFi.isConnected()) {
        delay(10);
    }
    Serial.println("WiFi Connected");
    delay(1000);

    client.onOpen(onOpen);
    client.onClose(onClose);
    client.onMessage(onMessage);
    client.onPing(onPing);
    client.onPong(onPong);
    client.onError(onError);
    client.begin(url);
}

void loop() {
    if (Serial.available()) {
        String data = Serial.readString();
        if (data.equals("close")) {
            client.close();
        } else if (data.equals("open")) {
            client.begin(url);
        } else {
            client.send(data);
        }
    }
}
````

### 2. ESP32 as Server
````c++
#include "WSServer.h"
#include "WiFi.h"

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
    if (Serial.available()) {
        String data = Serial.readString();
        for (auto& c : server.getClients()) {
            c.send(data);
        }
    }
}
````