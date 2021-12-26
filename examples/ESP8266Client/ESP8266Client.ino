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
